/*

* consumer.c
* ---
* Consumer thread implementation for Sub-Project-06.
*
* Workflow:
*
* ```
   dequeue()
  ```
* ```
       ↓
  ```
* process_packet()
*
* Features:
* * Multiple consumer support
* * Thread-safe packet consumption
* * Graceful shutdown
* * Future Group-E compatibility
*
* Notes:
* dequeue() is synchronized internally using
* pthread_rwlock_t in shared_buffer.c.
*
* Each packet is consumed exactly once because
* dequeue() acquires a WRITE lock before modifying:
*
* ```
    head
  ```
* ```
    count
  ```
*
* making the operation atomic.
  */

#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "consumer.h"
#include "packet_processor.h"
#include "metrics.h"

/*

* Global producer completion flag.
*
* Integration layer sets:
*
* ```
   producers_done = 1;
  ```
*
* after ALL producer threads have joined.
  */
volatile int producers_done = 0;

void *consumer_thread(void *arg)
{
    ConsumerArgs *args = (ConsumerArgs *)arg;

    if (args == NULL)
    {
        safe_log("[Consumer] ERROR: NULL argument received\n");
        return NULL;
    }

    safe_log("[Consumer-%d] Started\n",
             args->consumer_id);

    while (1)
    {
        DataUnit packet;

        int status =
            dequeue(
                args->buffer,
                &packet);

        switch (status)
        {
        case BUF_OK:

            if (args->metrics != NULL)
            {
                metrics_log_dequeue_by(args->metrics, args->buffer->count,
                                       packet.timestamp, args->consumer_id - 1);
            }

            process_packet(
                &packet,
                args->consumer_id);

            break;

        case BUF_EMPTY:
        {
            /*
             * Producers finished
             * AND no packets remain.
             */
            if (producers_done)
            {
                safe_log(
                    "[Consumer-%d] Buffer empty and producers finished. Exiting.\n",
                    args->consumer_id);

                safe_log(
                    "[Consumer-%d] Shutdown complete\n",
                    args->consumer_id);

                return NULL;
            }

            /*
             * Avoid busy waiting.
             * Sleep for 100 ms.
             */
            struct timespec ts;

            ts.tv_sec = 0;
            ts.tv_nsec = 250000000L;

            nanosleep(&ts, NULL);

            break;
        }

        case BUF_ERR:
        default:

            safe_log(
                "[Consumer-%d] ERROR: dequeue() failed\n",
                args->consumer_id);

            safe_log(
                "[Consumer-%d] Shutdown complete\n",
                args->consumer_id);

            return NULL;
        }
    }

    return NULL;
}