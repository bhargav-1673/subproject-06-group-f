#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "shared_buffer.h"
#include "producer.h"

int main()
{
    SharedBuffer buffer;

    pthread_t producer_threads[4];

    ProducerArgs producer_args[4];

    buffer_init(&buffer);

    /* Hyderabad */

    producer_args[0].buffer = &buffer;
    producer_args[0].packets_to_generate = 5;
    producer_args[0].start_packet_id = 1000;

    strcpy(
        producer_args[0].source_name,
        "Hyderabad");

    /* Mumbai */

    producer_args[1].buffer = &buffer;
    producer_args[1].packets_to_generate = 5;
    producer_args[1].start_packet_id = 2000;

    strcpy(
        producer_args[1].source_name,
        "Mumbai");

    /* Delhi */

    producer_args[2].buffer = &buffer;
    producer_args[2].packets_to_generate = 5;
    producer_args[2].start_packet_id = 3000;

    strcpy(
        producer_args[2].source_name,
        "Delhi");

    /* Bangalore */

    producer_args[3].buffer = &buffer;
    producer_args[3].packets_to_generate = 5;
    producer_args[3].start_packet_id = 4000;

    strcpy(
        producer_args[3].source_name,
        "Bangalore");

    /* Create Producer Threads */

    for(int i = 0; i < 4; i++)
    {
        pthread_create(
            &producer_threads[i],
            NULL,
            producer_thread,
            &producer_args[i]);
    }

    /* Wait for Completion */

    for(int i = 0; i < 4; i++)
    {
        pthread_join(
            producer_threads[i],
            NULL);
    }

    printf("\n=====================================\n");
    printf("MULTI-PRODUCER TEST COMPLETE\n");
    printf("Packets in Buffer = %d\n",
           buffer.count);
    printf("=====================================\n");

    buffer_destroy(&buffer);

    return 0;
}