#ifndef CONSUMER_H
#define CONSUMER_H

#include "shared_buffer.h"

struct MetricsTracker;

/*
 * Completion flag.
 *
 * Defined in consumer.c
 * Referenced by integration layer.
 *
 * Set to 1 after all producers finish.
 */
extern volatile int producers_done;

/*
 * Arguments passed to each consumer thread.
 */
typedef struct
{
    SharedBuffer *buffer;

    int consumer_id;
    struct MetricsTracker *metrics;

} ConsumerArgs;

/*
 * Consumer thread function.
 */
void *consumer_thread(void *arg);

#endif /* CONSUMER_H */