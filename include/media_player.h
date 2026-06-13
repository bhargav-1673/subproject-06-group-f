#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <pthread.h>
#include "shared_buffer.h"

/* ─────────────────────────────────────────────
 * MediaPlayerConfig
 * CLI-driven configuration for the integration
 * orchestrator. Passed into media_player_run().
 * ───────────────────────────────────────────── */
typedef struct {
    int num_producers;      /* Number of producer threads to spawn      */
    int num_consumers;      /* Number of consumer threads to spawn       */
    int packets_per_producer; /* Packets each producer will enqueue      */
} MediaPlayerConfig;

/* ─────────────────────────────────────────────
 * MediaPlayerContext
 * Internal runtime state of the orchestrator.
 * Holds the shared buffer, producer-done signal
 * (cond var + mutex), and thread handle arrays.
 * ───────────────────────────────────────────── */
typedef struct {
    SharedBuffer        buffer;

    /* Producer-done signal — used by main thread
     * to notify consumers after all producers
     * have been joined. Consumers wait on this
     * in addition to the BUF_EMPTY check.       */
    pthread_cond_t      producers_done_cond;
    pthread_mutex_t     producers_done_mutex;
    int                 producers_done;         /* 0 = running, 1 = done */

    /* Thread handle arrays (heap-allocated)     */
    pthread_t          *producer_threads;
    pthread_t          *consumer_threads;
} MediaPlayerContext;

/* ─────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────── */

/*
 * media_player_init()
 * Initializes the MediaPlayerContext:
 *   - buffer_init()
 *   - pthread_cond_init / pthread_mutex_init
 *   - allocates thread handle arrays
 * Returns 0 on success, -1 on failure.
 */
int media_player_init(MediaPlayerContext *ctx, const MediaPlayerConfig *cfg);

/*
 * media_player_run()
 * Spawns producer and consumer threads,
 * joins producers, signals consumers via
 * cond var, joins consumers.
 * Returns 0 on success, -1 on failure.
 */
int media_player_run(MediaPlayerContext *ctx, const MediaPlayerConfig *cfg);

/*
 * media_player_destroy()
 * Cleans up all resources:
 *   - buffer_destroy()
 *   - pthread_cond_destroy / pthread_mutex_destroy
 *   - frees thread handle arrays
 */
void media_player_destroy(MediaPlayerContext *ctx);

/*
 * media_player_print_summary()
 * Prints final packet count stats to stdout.
 */
void media_player_print_summary(const MediaPlayerConfig *cfg, int packets_consumed);

#endif /* MEDIA_PLAYER_H */