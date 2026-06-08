#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "producer.h"

DataUnit generate_packet(
        int packet_id,
        const char *source)
{
    DataUnit packet;

    memset(&packet, 0, sizeof(DataUnit));

    packet.packet_id = packet_id;

    strncpy(
        packet.source,
        source,
        sizeof(packet.source) - 1);

    packet.source[sizeof(packet.source) - 1] = '\0';

    snprintf(
        packet.data,
        sizeof(packet.data),
        "Packet-%d generated from %s",
        packet_id,
        source);

    packet.size = strlen(packet.data);

    packet.timestamp = time(NULL);

    return packet;
}

void *producer_thread(void *arg)
{
    ProducerArgs *args =
        (ProducerArgs *)arg;

    for(int i = 0;
        i < args->packets_to_generate;
        i++)
    {
        DataUnit packet =
            generate_packet(
                args->start_packet_id + i,
                args->source_name);

        int status =
            enqueue(
                args->buffer,
                &packet);

        if(status == BUF_OK)
        {
            printf(
                "[Producer %s] Packet %d Enqueued\n",
                args->source_name,
                packet.packet_id);
        }
        else if(status == BUF_FULL)
        {
            printf(
                "[Producer %s] BUFFER FULL "
                "Dropping Packet %d\n",
                args->source_name,
                packet.packet_id);
        }
        else
        {
            printf(
                "[Producer %s] ERROR while enqueue\n",
                args->source_name);
        }

        sleep(1);
    }

    pthread_exit(NULL);
}