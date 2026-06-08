#include <stdio.h>
#include <pthread.h>

#include "shared_buffer.h"
#include "producer.h"

int main()
{
    SharedBuffer buffer;

    pthread_t producer;

    ProducerArgs args;

    buffer_init(&buffer);

    args.buffer = &buffer;

    args.packets_to_generate = 10;

    snprintf(
        args.source_name,
        sizeof(args.source_name),
        "Hyderabad");

    pthread_create(
        &producer,
        NULL,
        producer_thread,
        &args);

    pthread_join(
        producer,
        NULL);

    printf(
        "\nPackets in Buffer = %d\n",
        buffer.count);

    buffer_destroy(&buffer);

    return 0;
}