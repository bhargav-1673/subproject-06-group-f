/*
 * producer.h
 * ----------
 * Producer thread interface for Sub-Project-06.
 *
 * producer_thread() generates packets via generate_packet()
 * (from packet_generator.h) and pushes them into the shared
 * buffer via enqueue(). It has no knowledge of the consumer.
 */

#ifndef PRODUCER_H
#define PRODUCER_H

#include "shared_buffer.h"
#include "packet_generator.h"

typedef struct
{
    SharedBuffer *buffer;
    char          source_name[32];
    int           packets_to_generate;
    int           start_packet_id;

} ProducerArgs;

void *producer_thread(void *arg);

#endif /* PRODUCER_H */