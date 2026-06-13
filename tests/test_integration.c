/*
 * test_integration.c
 * ------------------
 * End-to-end integration test for Sub-Project-06.
 *
 * Tests the full producer-consumer pipeline via the
 * media_player orchestration layer:
 *
 *   Producer Threads
 *        |
 *        v
 *   SharedBuffer (circular, rwlock)
 *        |
 *        v
 *   Consumer Threads
 *
 * Test Cases:
 *   1. Basic pipeline    : 1 producer,  1 consumer,  10 packets
 *   2. Standard pipeline : 4 producers, 4 consumers, 20 packets each
 *   3. More producers    : 6 producers, 2 consumers, 15 packets each
 *   4. More consumers    : 2 producers, 6 consumers, 15 packets each
 *
 * Pass Condition:
 *   All packets produced are consumed.
 *   Buffer is empty after all threads exit.
 *
 * Build:
 *   gcc -Wall -Wextra -O2 -pthread -Iinclude \
 *       tests/test_integration.c              \
 *       src/integration/media_player.c        \
 *       src/buffer/shared_buffer.c            \
 *       src/producer/producer.c               \
 *       src/producer/packet_generator.c       \
 *       src/consumer/consumer.c               \
 *       src/consumer/packet_processor.c       \
 *       -o build/test_integration
 *
 * Usage:
 *   ./build/test_integration
 */

#include <stdio.h>
#include <stdlib.h>

#include "media_player.h"
#include "shared_buffer.h"
#include "consumer.h"

/* ─────────────────────────────────────────────────────────────
 * Test runner
 * ───────────────────────────────────────────────────────────── */
static int run_test(
        const char *test_name,
        int num_producers,
        int num_consumers,
        int packets_per_producer)
{
    printf("\n");
    printf("========================================\n");
    printf(" %s\n", test_name);
    printf("========================================\n");
    printf("Producers : %d\n", num_producers);
    printf("Consumers : %d\n", num_consumers);
    printf("Packets   : %d each (%d total)\n",
           packets_per_producer,
           num_producers * packets_per_producer);
    printf("Buffer    : %d slots\n", BUFFER_SIZE);
    printf("----------------------------------------\n\n");

    MediaPlayerConfig cfg;
    cfg.num_producers        = num_producers;
    cfg.num_consumers        = num_consumers;
    cfg.packets_per_producer = packets_per_producer;

    MediaPlayerContext ctx;

    /* Init */
    if (media_player_init(&ctx, &cfg) != 0) {
        fprintf(stderr, "[%s] FAIL — media_player_init() failed\n", test_name);
        return -1;
    }

    /* Run */
    if (media_player_run(&ctx, &cfg) != 0) {
        fprintf(stderr, "[%s] FAIL — media_player_run() failed\n", test_name);
        media_player_destroy(&ctx);
        return -1;
    }

    /* Evaluate */
    int total_produced   = num_producers * packets_per_producer;
    int remaining        = ctx.buffer.count;
    int packets_consumed = total_produced - remaining;

    media_player_print_summary(&cfg, packets_consumed);
    media_player_destroy(&ctx);

    if (packets_consumed == total_produced) {
        printf("[%s] PASS\n", test_name);
        return 0;
    } else {
        printf("[%s] FAIL — produced=%d consumed=%d remaining=%d\n",
               test_name, total_produced, packets_consumed, remaining);
        return -1;
    }
}

/* ─────────────────────────────────────────────────────────────
 * main()
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  C-DAC Sub-Project-06 | Group-F        \n");
    printf("  Integration Test Suite                \n");
    printf("========================================\n");

    int passed = 0;
    int failed = 0;

    /* ── Test 1: Basic pipeline ── */
    if (run_test("TEST 1 — Basic Pipeline",
                 1, 1, 10) == 0)
        passed++;
    else
        failed++;

    /* ── Test 2: Standard pipeline (4P + 4C) ──
     * Primary integration validation.
     * 80 total packets, buffer size 20.
     * Producers will hit BUF_FULL — dropped
     * packets are expected and handled.        */
    if (run_test("TEST 2 — Standard Pipeline (4P + 4C)",
                 4, 4, 20) == 0)
        passed++;
    else
        failed++;

    /* ── Test 3: Producer-heavy ── */
    if (run_test("TEST 3 — Producer-Heavy (6P + 2C)",
                 6, 2, 15) == 0)
        passed++;
    else
        failed++;

    /* ── Test 4: Consumer-heavy ── */
    if (run_test("TEST 4 — Consumer-Heavy (2P + 6C)",
                 2, 6, 15) == 0)
        passed++;
    else
        failed++;

    /* ── Final Summary ── */
    printf("\n");
    printf("========================================\n");
    printf("        INTEGRATION TEST SUMMARY        \n");
    printf("========================================\n");
    printf("Tests Passed : %d\n", passed);
    printf("Tests Failed : %d\n", failed);
    printf("Total        : %d\n", passed + failed);
    printf("Result       : %s\n", (failed == 0) ? "ALL PASSED" : "SOME FAILED");
    printf("========================================\n\n");

    return (failed == 0) ? 0 : 1;
}