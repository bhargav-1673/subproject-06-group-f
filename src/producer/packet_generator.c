/*
 * packet_generator.c
 * ------------------
 * Simulated telecom packet generator for Sub-Project-06.
 *
 * This file is intentionally isolated from producer logic.
 * To switch to real telecom data: replace this file only.
 * producer.c and producer_thread() do not need any changes.
 *
 * Author  : Jagabathuni V M Bhargav (Group Lead, Group-F)
 * Build   : compiled together with producer.c and shared_buffer.c
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "packet_generator.h"

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

    packet.size      = (int)strlen(packet.data);
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    packet.timestamp = (time_t)((long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

    return packet;
}