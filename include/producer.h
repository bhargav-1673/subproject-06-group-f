#ifndef PRODUCER_H
#define PRODUCER_H

#include "shared_buffer.h"

typedef struct
{
    SharedBuffer *buffer;

    char source_name[32];

    int packets_to_generate;

    int start_packet_id;

} ProducerArgs;

DataUnit generate_packet(
        int packet_id,
        const char *source);

void *producer_thread(
        void *arg);

#endif