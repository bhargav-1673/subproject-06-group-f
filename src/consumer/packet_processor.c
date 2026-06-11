/*
 * packet_processor.c
 * ------------------
 * Swappable packet processing module.
 *
 * Current Version:
 *   Displays packet metadata.
 *
 * Future Versions:
 *   Group-E may replace this entire file.
 */

#include <stdio.h>
#include <time.h>

#include "packet_processor.h"

void process_packet(
        const DataUnit *packet,
        int consumer_id)
{
    if (packet == NULL)
        return;

    printf(
        "[Consumer-%d] Processed "
        "Packet-%d | Source=%s | Size=%d | Timestamp=%ld\n",
        consumer_id,
        packet->packet_id,
        packet->source,
        packet->size,
        (long)packet->timestamp);

    printf(
        "    Payload: %s\n",
        packet->data);
}