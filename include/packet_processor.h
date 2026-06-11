#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

#include "shared_buffer.h"

/*
 * process_packet()
 * ----------------
 * Consumer-side processing function.
 *
 * Current implementation:
 *   Prints packet information.
 *
 * Future (Group-E):
 *   Replace this implementation with:
 *     - Telecom analytics
 *     - MPI transmission
 *     - Database insertion
 *     - Packet inspection
 *
 * consumer.c should NEVER need modification.
 */

void process_packet(
        const DataUnit *packet,
        int consumer_id);

#endif /* PACKET_PROCESSOR_H */