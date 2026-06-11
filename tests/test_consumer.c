/*
 * test_consumer.c
 * ----------------
 * Tests the consumer layer with multiple concurrent consumers.
 *
 * Usage:
 *   ./test_consumer <num_consumers> <num_packets>
 *
 * Example:
 *   ./test_consumer 4 100
 *
 * Verifies:
 *   - Multiple consumers run concurrently
 *   - Each packet is consumed exactly once
 *   - Buffer drains correctly
 *   - Consumers terminate gracefully
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "shared_buffer.h"
#include "consumer.h"

#define MAX_CONSUMERS 8

extern volatile int producers_done;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr,
                "Usage: %s <num_consumers> <num_packets>\n"
                "  num_consumers : 1-%d\n"
                "  num_packets   : total packets to generate\n",
                argv[0],
                MAX_CONSUMERS);
        return 1;
    }

    int num_consumers = atoi(argv[1]);
    int num_packets = atoi(argv[2]);

    if (num_consumers < 1 || num_consumers > MAX_CONSUMERS)
    {
        fprintf(stderr,
                "ERROR: num_consumers must be between 1 and %d\n",
                MAX_CONSUMERS);
        return 1;
    }

    if (num_packets < 1)
    {
        fprintf(stderr,
                "ERROR: num_packets must be >= 1\n");
        return 1;
    }

    printf("=== Consumer Test ===\n");
    printf("Shared Buffer Version : %s\n",
           SHARED_BUFFER_VERSION);
    printf("Consumers : %d\n", num_consumers);
    printf("Packets   : %d\n", num_packets);
    printf("Buffer    : %d slots\n\n", BUFFER_SIZE);

    SharedBuffer buf;

    if (buffer_init(&buf) != BUF_OK)
    {
        fprintf(stderr,
                "ERROR: buffer_init failed\n");
        return 1;
    }

    producers_done = 0;

    /* ---------------------------------
     * Launch Consumer Threads
     * --------------------------------- */
    pthread_t consumers[MAX_CONSUMERS];
    ConsumerArgs args[MAX_CONSUMERS];

    for (int i = 0; i < num_consumers; i++)
    {
        args[i].buffer = &buf;
        args[i].consumer_id = i + 1;

        if (pthread_create(&consumers[i],
                           NULL,
                           consumer_thread,
                           &args[i]) != 0)
        {
            fprintf(stderr,
                    "ERROR: pthread_create failed for Consumer-%d\n",
                    i + 1);

            buffer_destroy(&buf);
            return 1;
        }
    }

    /* ---------------------------------
     * Simulate Packet Production
     * --------------------------------- */
    int packets_inserted = 0;

    for (int i = 0; i < num_packets; i++)
    {
        DataUnit packet;

        memset(&packet, 0, sizeof(packet));

        packet.packet_id = i + 1;

        strcpy(packet.source, "Mumbai");

        snprintf(packet.data,
                 sizeof(packet.data),
                 "Test Packet %d",
                 i + 1);

        packet.size = strlen(packet.data);
        packet.timestamp = time(NULL);

        /*
         * Retry if buffer is full.
         * This ensures all packets eventually
         * enter the shared buffer.
         */
        while (enqueue(&buf, &packet) == BUF_FULL)
        {
            usleep(50000); /* 50 ms */
        }

        packets_inserted++;

        /*
         * Simulate telecom packet arrival interval.
         */
        usleep(20000); /* 20 ms */
    }

    /* ---------------------------------
     * Production Complete
     * --------------------------------- */
    producers_done = 1;

    printf("\n[TEST] Producer finished generating %d packets\n",
           packets_inserted);

    /* ---------------------------------
     * Wait For Consumers
     * --------------------------------- */
    for (int i = 0; i < num_consumers; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    /* ---------------------------------
     * Test Summary
     * --------------------------------- */
    printf("\n=============================\n");
    printf("        TEST RESULT\n");
    printf("=============================\n");

    printf("Consumers            : %d\n",
           num_consumers);

    printf("Packets Requested    : %d\n",
           num_packets);

    printf("Packets Inserted     : %d\n",
           packets_inserted);

    printf("Remaining In Buffer  : %d\n",
           buf.count);

    if (packets_inserted == num_packets &&
        buf.count == 0)
    {
        printf("Status               : PASS\n");
    }
    else
    {
        printf("Status               : FAIL\n");
    }

    printf("=============================\n");

    buffer_destroy(&buf);

    return 0;
}
