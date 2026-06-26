#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "media_player.h"
#include "shared_buffer.h"
#include "producer.h"
#include "consumer.h"

/* ─────────────────────────────────────────────────────────────
 * media_player_init()
 * ───────────────────────────────────────────────────────────── */
int media_player_init(MediaPlayerContext *ctx, const MediaPlayerConfig *cfg)
{
    /* Initialize shared buffer */
    buffer_init(&ctx->buffer);

    /* Initialize metrics tracker */
    metrics_init(&ctx->metrics);

    /* Initialize producer-done cond var + mutex */
    if (pthread_mutex_init(&ctx->producers_done_mutex, NULL) != 0) {
        fprintf(stderr, "[media_player_init] pthread_mutex_init failed\n");
        return -1;
    }
    if (pthread_cond_init(&ctx->producers_done_cond, NULL) != 0) {
        fprintf(stderr, "[media_player_init] pthread_cond_init failed\n");
        pthread_mutex_destroy(&ctx->producers_done_mutex);
        return -1;
    }

    ctx->producers_done = 0;

    /* Allocate thread handle arrays */
    ctx->producer_threads = malloc(sizeof(pthread_t) * cfg->num_producers);
    if (!ctx->producer_threads) {
        fprintf(stderr, "[media_player_init] malloc failed (producer_threads)\n");
        pthread_cond_destroy(&ctx->producers_done_cond);
        pthread_mutex_destroy(&ctx->producers_done_mutex);
        return -1;
    }

    ctx->consumer_threads = malloc(sizeof(pthread_t) * cfg->num_consumers);
    if (!ctx->consumer_threads) {
        fprintf(stderr, "[media_player_init] malloc failed (consumer_threads)\n");
        free(ctx->producer_threads);
        pthread_cond_destroy(&ctx->producers_done_cond);
        pthread_mutex_destroy(&ctx->producers_done_mutex);
        return -1;
    }

    return 0;
}

/* ─────────────────────────────────────────────────────────────
 * media_player_run()
 * ───────────────────────────────────────────────────────────── */
int media_player_run(MediaPlayerContext *ctx, const MediaPlayerConfig *cfg)
{
    int i;

    /* ── Allocate ProducerArgs ── */
    ProducerArgs *p_args = malloc(sizeof(ProducerArgs) * cfg->num_producers);
    if (!p_args) {
        fprintf(stderr, "[media_player_run] malloc failed (ProducerArgs)\n");
        return -1;
    }

    /* ── Allocate ConsumerArgs ── */
    ConsumerArgs *c_args = malloc(sizeof(ConsumerArgs) * cfg->num_consumers);
    if (!c_args) {
        fprintf(stderr, "[media_player_run] malloc failed (ConsumerArgs)\n");
        free(p_args);
        return -1;
    }

    /* Start metrics tracking */
    metrics_start(&ctx->metrics);

    /* ── Spawn consumer threads first ──
     * Consumers start waiting before any data arrives.
     * Mirrors hyPACK: renderer is ready before the network
     * thread begins streaming frames into the in-buffer.
     *
     * producers_done is extern volatile int defined in consumer.c.
     * Set to 0 here before spawning — no field in ConsumerArgs.   */
    producers_done = 0;

    printf("[media_player] Spawning %d consumer thread(s)...\n", cfg->num_consumers);
    for (i = 0; i < cfg->num_consumers; i++) {
        c_args[i].buffer      = &ctx->buffer;
        c_args[i].consumer_id = i + 1;
        c_args[i].metrics     = &ctx->metrics;

        if (pthread_create(&ctx->consumer_threads[i], NULL,
                           consumer_thread, &c_args[i]) != 0) {
            fprintf(stderr, "[media_player_run] pthread_create failed (consumer %d)\n", i + 1);
            free(p_args);
            free(c_args);
            return -1;
        }
    }

    /* ── Spawn producer threads ──
     * Mirrors hyPACK: network thread starts streaming
     * data into the in-buffer after renderer is ready.
     * Site names map to telecom scenario city identifiers. */
    const char *site_names[] = {
        "Hyderabad", "Delhi", "Mumbai", "Chennai",
        "Bangalore", "Kolkata", "Pune", "Ahmedabad"
    };
    int max_sites = (int)(sizeof(site_names) / sizeof(site_names[0]));

    printf("[media_player] Spawning %d producer thread(s)...\n", cfg->num_producers);
    for (i = 0; i < cfg->num_producers; i++) {
        p_args[i].buffer              = &ctx->buffer;
        p_args[i].packets_to_generate = cfg->packets_per_producer;
        p_args[i].producer_id         = i;   /* 0-based; matches producer_enqueued[i] */
        /* start_packet_id offset ensures globally unique IDs across producers */
        p_args[i].start_packet_id     = (i * cfg->packets_per_producer) + 1;
        p_args[i].metrics             = &ctx->metrics;
        snprintf(p_args[i].source_name, sizeof(p_args[i].source_name),
                 "%s", site_names[i % max_sites]);

        if (pthread_create(&ctx->producer_threads[i], NULL,
                           producer_thread, &p_args[i]) != 0) {
            fprintf(stderr, "[media_player_run] pthread_create failed (producer %d)\n", i + 1);
            free(p_args);
            free(c_args);
            return -1;
        }
    }

    /* ── Join all producer threads ── */
    printf("[media_player] Waiting for producer threads to complete...\n");
    for (i = 0; i < cfg->num_producers; i++) {
        pthread_join(ctx->producer_threads[i], NULL);
    }
    printf("[media_player] All producer threads completed.\n");

    /* ── Signal consumers: producers are done ──
     * Use pthread_cond_broadcast so ALL waiting
     * consumer threads are woken simultaneously.
     * This satisfies the hyPACK cond var requirement
     * while keeping consumer.c frozen.               */
    pthread_mutex_lock(&ctx->producers_done_mutex);
    ctx->producers_done = 1;  /* integration layer state  */
    producers_done      = 1;  /* global read by consumer.c */
    pthread_cond_broadcast(&ctx->producers_done_cond);
    pthread_mutex_unlock(&ctx->producers_done_mutex);
    printf("[media_player] Producer-done signal broadcast to consumers.\n");

    /* ── Join all consumer threads ── */
    printf("[media_player] Waiting for consumer threads to complete...\n");
    for (i = 0; i < cfg->num_consumers; i++) {
        pthread_join(ctx->consumer_threads[i], NULL);
    }
    printf("[media_player] All consumer threads completed.\n");

    /* Stop metrics tracking and print report */
    metrics_stop(&ctx->metrics);
    metrics_print_report(&ctx->metrics, cfg->num_producers, cfg->num_consumers,
                         cfg->num_producers * cfg->packets_per_producer);

    free(p_args);
    free(c_args);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
 * media_player_destroy()
 * ───────────────────────────────────────────────────────────── */
void media_player_destroy(MediaPlayerContext *ctx)
{
    buffer_destroy(&ctx->buffer);
    pthread_cond_destroy(&ctx->producers_done_cond);
    pthread_mutex_destroy(&ctx->producers_done_mutex);
    free(ctx->producer_threads);
    free(ctx->consumer_threads);
    ctx->producer_threads = NULL;
    ctx->consumer_threads = NULL;
}

/* ─────────────────────────────────────────────────────────────
 * media_player_print_summary()
 * ───────────────────────────────────────────────────────────── */
void media_player_print_summary(const MediaPlayerConfig *cfg, int packets_consumed)
{
    int total_produced = cfg->num_producers * cfg->packets_per_producer;

    printf("\n");
    printf("========================================\n");
    printf("        INTEGRATION TEST RESULT         \n");
    printf("========================================\n");
    printf("Producers            : %d\n", cfg->num_producers);
    printf("Consumers            : %d\n", cfg->num_consumers);
    printf("Packets Per Producer : %d\n", cfg->packets_per_producer);
    printf("Total Produced       : %d\n", total_produced);
    printf("Total Consumed       : %d\n", packets_consumed);
    printf("Remaining In Buffer  : %d\n", total_produced - packets_consumed);
    printf("Status               : %s\n",
           (packets_consumed == total_produced) ? "PASS" : "FAIL");
    printf("========================================\n");
}