/*
 * shared_buffer.h
 * ---------------
 * Central shared header for Group-F, Sub-Project-06
 * C-DAC MPPLAB Internship, June 2026
 *
 * Project:
 *   Multi-Threaded (POSIX) Implementation of Data Extraction
 *   from a Telecom Data Extraction Server
 *
 * Description:
 *   This file defines:
 *     (1) DataUnit      - Generic packet/data unit used throughout the system
 *     (2) SharedBuffer  - Circular buffer protected by a POSIX RW Lock
 *     (3) Public APIs   - buffer_init(), enqueue(), dequeue(), buffer_destroy()
 *
 * Design Goals:
 *   - Generic enough for telecom packet processing
 *   - Generic enough for media streaming applications
 *   - Reusable by future MPI-based integrations
 *   - Independent of producer/consumer implementation details
 *
 * Usage:
 *   ALL modules must include this file.
 *   Direct access to internal buffer state is strongly discouraged.
 *   Always use enqueue() and dequeue() APIs.
 *
 * Future Integration:
 *   Group-E (MPI Layer) should use the public APIs defined here
 *   rather than modifying the internal implementation.
 *
 * Build:
 *   gcc -pthread -O2 your_file.c shared_buffer.c -o output
 */

#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H
#define _XOPEN_SOURCE 700

#include <pthread.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================================================
     * VERSION
     * ========================================================= */

#define SHARED_BUFFER_VERSION "1.0"

    /* =========================================================
     * SECTION 1 - CONFIGURATION CONSTANTS
     *
     * BUFFER_SIZE can be overridden before including this file:
     *
     *   #define BUFFER_SIZE 64
     *   #include "shared_buffer.h"
     *
     * ========================================================= */

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 20
#endif

#define SOURCE_LEN 32
#define DATA_LEN 1024

    /* =========================================================
     * SECTION 2 - RETURN CODES
     * ========================================================= */

#define BUF_OK 0
#define BUF_FULL -1
#define BUF_EMPTY -2
#define BUF_ERR -3

    /* =========================================================
     * SECTION 3 - DataUnit
     *
     * Generic packet representation used throughout the system.
     *
     * Telecom Example:
     *   packet_id = sequence number
     *   source    = city/site identifier
     *   data      = raw telecom payload
     *
     * Media Example:
     *   packet_id = frame/chunk number
     *   source    = stream identifier
     *   data      = frame/chunk payload
     *
     * Avoid adding application-specific fields here.
     * ========================================================= */

    typedef struct
    {
        int packet_id;           /* Unique packet/frame identifier      */
        char source[SOURCE_LEN]; /* Origin site, city, stream, etc.     */
        char data[DATA_LEN];     /* Payload data                        */
        int size;                /* Valid bytes stored in data[]        */
        time_t timestamp;        /* Packet creation time                */

    } DataUnit;

    /* =========================================================
     * SECTION 4 - SharedBuffer
     *
     * Circular buffer (ring buffer) implementation.
     *
     * Buffer State:
     *   head  -> next element to remove
     *   tail  -> next free position to insert
     *   count -> current number of stored packets
     *
     * Synchronization:
     *   Protected by pthread_rwlock_t.
     *
     * NOTE:
     *   Although the internal fields are visible for transparency,
     *   application code should never modify them directly.
     * ========================================================= */

    typedef struct
    {
        DataUnit slots[BUFFER_SIZE];

        int head;
        int tail;
        int count;

        pthread_rwlock_t lock;

    } SharedBuffer;

    /* =========================================================
     * SECTION 5 - PUBLIC API
     * ========================================================= */

    /*
     * buffer_init()
     * ---------------------------------------------------------
     * Initializes:
     *   - head
     *   - tail
     *   - count
     *   - rwlock
     *
     * Must be called before any producer or consumer threads
     * are created.
     *
     * Returns:
     *   BUF_OK  on success
     *   BUF_ERR on failure
     */
    int buffer_init(SharedBuffer *buf);

    /*
     * enqueue()
     * ---------------------------------------------------------
     * Inserts a packet into the circular buffer.
     *
     * Synchronization:
     *   Uses pthread_rwlock_wrlock()
     *   because buffer state is modified.
     *
     * Thread Safety:
     *   Safe for multiple producer threads.
     *
     * Parameters:
     *   buf    - target shared buffer
     *   packet - packet to insert
     *
     * Returns:
     *   BUF_OK    on success
     *   BUF_FULL  if buffer has no free slots
     */
    int enqueue(
        SharedBuffer *buf,
        const DataUnit *packet);

    /*
     * dequeue()
     * ---------------------------------------------------------
     * Removes the oldest packet from the buffer.
     *
     * Synchronization:
     *   Uses pthread_rwlock_wrlock()
     *   because head and count are modified.
     *
     * Thread Safety:
     *   Safe for multiple consumer threads.
     *
     * Parameters:
     *   buf - source shared buffer
     *   out - destination packet
     *
     * Returns:
     *   BUF_OK      on success
     *   BUF_EMPTY   if no packets are available
     */
    int dequeue(
        SharedBuffer *buf,
        DataUnit *out);

    /*
     * Optional Future Extension
     *
     * A read-only API may use:
     *
     *   pthread_rwlock_rdlock()
     *
     * Example:
     *   int peek(SharedBuffer *buf, DataUnit *out);
     *
     * This has intentionally been left out of Version 1.0
     * to keep the API minimal.
     */

    /*
     * buffer_destroy()
     * ---------------------------------------------------------
     * Destroys the rwlock associated with the buffer.
     *
     * Must be called after:
     *   - all producer threads exit
     *   - all consumer threads exit
     *   - all pthread_join() calls complete
     *
     * Failure to call this may result in resource leaks.
     */
    void buffer_destroy(SharedBuffer *buf);

#ifdef __cplusplus
}
#endif

#endif /* SHARED_BUFFER_H */
