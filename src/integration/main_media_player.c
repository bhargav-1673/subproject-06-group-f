/*
 * main_media_player.c
 * -------------------
 * Entry point for the standalone media_player binary.
 *
 * Kept separate from media_player.c so that the orchestration
 * library functions can be linked into test_integration.c
 * without a duplicate main() conflict.
 *
 * Usage:
 *   ./build/media_player <num_producers> <num_consumers> <packets_each>
 *
 * Example:
 *   ./build/media_player 4 4 20
 */

#include <stdio.h>
#include <stdlib.h>

#include "media_player.h"
#include "shared_buffer.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <num_producers> <num_consumers> <packets_each>\n",
                argv[0]);
        return 1;
    }

    MediaPlayerConfig cfg;
    cfg.num_producers        = atoi(argv[1]);
    cfg.num_consumers        = atoi(argv[2]);
    cfg.packets_per_producer = atoi(argv[3]);

    if (cfg.num_producers <= 0 || cfg.num_consumers <= 0 ||
        cfg.packets_per_producer <= 0) {
        fprintf(stderr, "[main] All arguments must be positive integers.\n");
        return 1;
    }

    printf("========================================\n");
    printf("  C-DAC Sub-Project-06 | Group-F        \n");
    printf("  Multi-Threaded Telecom Data Extraction \n");
    printf("========================================\n");
    printf("Producers : %d\n", cfg.num_producers);
    printf("Consumers : %d\n", cfg.num_consumers);
    printf("Packets   : %d each (%d total)\n",
           cfg.packets_per_producer,
           cfg.num_producers * cfg.packets_per_producer);
    printf("Buffer    : %d slots\n", BUFFER_SIZE);
    printf("========================================\n\n");

    MediaPlayerContext ctx;

    if (media_player_init(&ctx, &cfg) != 0) {
        fprintf(stderr, "[main] Initialization failed.\n");
        return 1;
    }

    if (media_player_run(&ctx, &cfg) != 0) {
        fprintf(stderr, "[main] Run failed.\n");
        media_player_destroy(&ctx);
        return 1;
    }

    /* packets_consumed = total - remaining in buffer */
    int total_produced   = cfg.num_producers * cfg.packets_per_producer;
    int remaining        = ctx.buffer.count;
    int packets_consumed = total_produced - remaining;

    media_player_print_summary(&cfg, packets_consumed);
    media_player_destroy(&ctx);

    return (packets_consumed == total_produced) ? 0 : 1;
}