/*
 * packet_generator.h
 * ------------------
 * Public interface for packet generation.
 *
 * This file is intentionally separate from producer.h so that
 * generate_packet() can be swapped with real telecom data later
 * without modifying producer_thread() at all.
 *
 * Group-E: replace packet_generator.c with your own implementation
 * that pulls real data — producer.c does not need to change.
 */

#ifndef PACKET_GENERATOR_H
#define PACKET_GENERATOR_H

#include "shared_buffer.h"

/*
 * generate_packet()
 * Creates and returns a DataUnit with simulated telecom data.
 *
 * packet_id : unique ID for this packet
 * source    : origin city or site name (max 31 chars)
 *
 * Returns   : fully populated DataUnit (by value)
 */
DataUnit generate_packet(
        int packet_id,
        const char *source);

#endif /* PACKET_GENERATOR_H */