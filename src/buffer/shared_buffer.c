/*
 * shared_buffer.c
 * ---------------
 * Implementation of the shared circular buffer for Group-F,
 * Sub-Project-06 (C-DAC MPPLAB Internship, June 2026)
 *
 * Description:
 *   Thread-safe circular buffer implementation using POSIX
 *   Read-Write Locks (pthread_rwlock_t).
 *
 * Synchronization Strategy:
 *   - enqueue() uses WRITE lock because it modifies buffer state.
 *   - dequeue() uses WRITE lock because it modifies head/count.
 *
 * This implementation is safe for:
 *   - Multiple Producers
 *   - Multiple Consumers
 *   - Future MPI + Pthreads integration (Group-E)
 *
 * Build:
 *   gcc -pthread -O2 -c shared_buffer.c -o shared_buffer.o
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "shared_buffer.h"

/* =========================================================
 * buffer_init
 * ========================================================= */
int buffer_init(SharedBuffer *buf)
{
    if (buf == NULL)
        return BUF_ERR;

    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;

    /* Clear storage */
    memset(buf->slots, 0, sizeof(buf->slots));

    /* Initialize RW lock */
    if (pthread_rwlock_init(&buf->lock, NULL) != 0)
    {
        fprintf(stderr,
                "[buffer_init] ERROR: pthread_rwlock_init failed\n");
        return BUF_ERR;
    }

    printf("[buffer_init] Shared buffer initialized (%d slots)\n",
           BUFFER_SIZE);

    return BUF_OK;
}

/* =========================================================
 * enqueue
 * Inserts a packet into the circular buffer.
 *
 * Returns:
 *   BUF_OK
 *   BUF_FULL
 *   BUF_ERR
 * ========================================================= */
int enqueue(
    SharedBuffer *buf,
    const DataUnit *packet)
{
    if (buf == NULL || packet == NULL)
        return BUF_ERR;

    if (pthread_rwlock_wrlock(&buf->lock) != 0)
        return BUF_ERR;

    /* Buffer full */
    if (buf->count == BUFFER_SIZE)
    {
        pthread_rwlock_unlock(&buf->lock);

        printf("[enqueue] BUFFER FULL | packet_id=%d | source=%s\n",
               packet->packet_id,
               packet->source);

        return BUF_FULL;
    }

    /* Copy packet into tail slot */
    buf->slots[buf->tail] = *packet;

    /* Advance tail (circular wrap-around) */
    buf->tail = (buf->tail + 1) % BUFFER_SIZE;
    buf->count++;

    int current_count = buf->count;

    pthread_rwlock_unlock(&buf->lock);

    printf("[enqueue] Thread %lu | packet_id=%d | source=%s | count=%d/%d\n",
           (unsigned long)pthread_self(),
           packet->packet_id,
           packet->source,
           current_count,
           BUFFER_SIZE);

    return BUF_OK;
}

/* =========================================================
 * dequeue
 * Removes the oldest packet from the circular buffer.
 *
 * Returns:
 *   BUF_OK
 *   BUF_EMPTY
 *   BUF_ERR
 *
 * NOTE:
 *   Uses WRITE lock because dequeue modifies:
 *     - head
 *     - count
 *
 * This guarantees correctness for multiple consumers.
 * ========================================================= */
int dequeue(
    SharedBuffer *buf,
    DataUnit *out)
{
    if (buf == NULL || out == NULL)
        return BUF_ERR;

    if (pthread_rwlock_wrlock(&buf->lock) != 0)
        return BUF_ERR;

    /* Buffer empty */
    if (buf->count == 0)
    {
        pthread_rwlock_unlock(&buf->lock);

        printf("[dequeue] BUFFER EMPTY | Thread %lu\n",
               (unsigned long)pthread_self());

        return BUF_EMPTY;
    }

    /* Copy oldest packet */
    *out = buf->slots[buf->head];

    /* Advance head (circular wrap-around) */
    buf->head = (buf->head + 1) % BUFFER_SIZE;
    buf->count--;

    int current_count = buf->count;

    pthread_rwlock_unlock(&buf->lock);

    printf("[dequeue] Thread %lu | packet_id=%d | source=%s | count=%d/%d\n",
           (unsigned long)pthread_self(),
           out->packet_id,
           out->source,
           current_count,
           BUFFER_SIZE);

    return BUF_OK;
}

/* =========================================================
 * buffer_destroy
 * ========================================================= */
void buffer_destroy(SharedBuffer *buf)
{
    if (buf == NULL)
        return;

    int ret = pthread_rwlock_destroy(&buf->lock);

    if (ret != 0)
    {
        fprintf(stderr,
                "[buffer_destroy] ERROR: pthread_rwlock_destroy failed (%d)\n",
                ret);
        return;
    }

    printf("[buffer_destroy] Shared buffer shutdown complete\n");
}
