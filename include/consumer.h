#ifndef CONSUMER_H
#define CONSUMER_H

#include "shared_buffer.h"

typedef struct
{
    SharedBuffer *buffer;

    int packets_to_consume;

} ConsumerArgs;

void process_packet(
        const DataUnit *packet);

void *consumer_thread(
        void *arg);

#endif