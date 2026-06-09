/*
 * producer.c
 * ----------
 * Producer thread implementation for Sub-Project-06.
 *
 * Each producer thread:
 *   1. Calls generate_packet() to create a DataUnit
 *   2. Calls enqueue() to push it into the shared buffer
 *   3. Sleeps briefly to simulate real telecom intervals
 *
 * generate_packet() lives in packet_generator.c — fully swappable.
 * This file does not need to change if the data source changes.
 *
 * Author  : Jagabathuni V M Bhargav (Group Lead, Group-F)
 * Build   : gcc -pthread -O2 producer.c packet_generator.c shared_buffer.c -o ...
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "producer.h"

void *producer_thread(void *arg)
{
    ProducerArgs *args = (ProducerArgs *)arg;

    for (int i = 0; i < args->packets_to_generate; i++)
    {
        DataUnit packet = generate_packet(
                args->start_packet_id + i,
                args->source_name);

        int status = enqueue(args->buffer, &packet);

        if (status == BUF_OK)
        {
            printf("[Producer %s] Packet %d enqueued\n",
                   args->source_name,
                   packet.packet_id);
        }
        else if (status == BUF_FULL)
        {
            printf("[Producer %s] BUFFER FULL — Packet %d dropped\n",
                   args->source_name,
                   packet.packet_id);
        }
        else
        {
            printf("[Producer %s] ERROR on enqueue (packet %d)\n",
                   args->source_name,
                   packet.packet_id);
        }

        sleep(1);
    }

    pthread_exit(NULL);
}