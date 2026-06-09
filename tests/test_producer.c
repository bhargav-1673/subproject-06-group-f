/*
 * test_producer.c
 * ---------------
 * Tests the producer layer against the shared buffer.
 *
 * Usage : ./test_producer <num_producers> <packets_each>
 * Example: ./test_producer 4 5
 *
 * - num_producers : number of concurrent producer threads (min 1, max 8)
 * - packets_each  : packets each producer generates
 *
 * City names are assigned round-robin from a fixed list.
 * Each producer gets a unique start_packet_id (offset by 1000).
 *
 * Compile:
 *   gcc -pthread -O2 test_producer.c shared_buffer.c producer.c packet_generator.c -o test_producer
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "shared_buffer.h"
#include "producer.h"

#define MAX_PRODUCERS 8

static const char *CITY_NAMES[] = {
    "Hyderabad", "Mumbai", "Delhi", "Bangalore",
    "Chennai",   "Kolkata", "Pune", "Jaipur"
};

int main(int argc, char *argv[])
{
    /* --- Parse arguments --- */
    if (argc != 3)
    {
        fprintf(stderr,
                "Usage: %s <num_producers> <packets_each>\n"
                "  num_producers : 1 to %d\n"
                "  packets_each  : packets per producer thread\n",
                argv[0], MAX_PRODUCERS);
        return 1;
    }

    int num_producers  = atoi(argv[1]);
    int packets_each   = atoi(argv[2]);

    if (num_producers < 1 || num_producers > MAX_PRODUCERS)
    {
        fprintf(stderr, "ERROR: num_producers must be 1-%d\n", MAX_PRODUCERS);
        return 1;
    }
    if (packets_each < 1)
    {
        fprintf(stderr, "ERROR: packets_each must be >= 1\n");
        return 1;
    }

    printf("=== Producer Test ===\n");
    printf("Producers : %d\n", num_producers);
    printf("Packets   : %d each (%d total)\n",
           packets_each, num_producers * packets_each);
    printf("Buffer    : %d slots\n\n", BUFFER_SIZE);

    /* --- Init buffer --- */
    SharedBuffer buf;
    if (buffer_init(&buf) != BUF_OK)
    {
        fprintf(stderr, "ERROR: buffer_init failed\n");
        return 1;
    }

    /* --- Set up producer args --- */
    ProducerArgs args[MAX_PRODUCERS];
    pthread_t    threads[MAX_PRODUCERS];

    for (int i = 0; i < num_producers; i++)
    {
        args[i].buffer              = &buf;
        args[i].packets_to_generate = packets_each;
        args[i].start_packet_id     = (i + 1) * 1000;

        /* assign city name round-robin */
        snprintf(args[i].source_name,
                 sizeof(args[i].source_name),
                 "%s", CITY_NAMES[i % MAX_PRODUCERS]);
    }

    /* --- Launch threads --- */
    for (int i = 0; i < num_producers; i++)
        pthread_create(&threads[i], NULL, producer_thread, &args[i]);

    /* --- Join threads --- */
    for (int i = 0; i < num_producers; i++)
        pthread_join(threads[i], NULL);

    /* --- Result --- */
    int expected = num_producers * packets_each;
    int actual   = buf.count;
    int dropped  = expected > BUFFER_SIZE ? expected - BUFFER_SIZE : 0;
    int expected_in_buf = expected - dropped;

    printf("\n--- Result ---\n");
    printf("Expected in buffer : %d\n", expected_in_buf);
    printf("Actual in buffer   : %d\n", actual);
    printf("Status             : %s\n",
           actual == expected_in_buf ? "PASS" : "FAIL");

    buffer_destroy(&buf);
    return 0;
}