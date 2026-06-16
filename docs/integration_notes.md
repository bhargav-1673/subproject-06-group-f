# Integration Notes

## Group-F → Group-E Handoff Document

**Sub-Project-06:** Multi-Threaded Telecom Data Extraction Server
**Group-F Lead:** Jagabathuni V M Bhargav
**Date:** 2026-06-13

---

## Overview

This document describes the stable APIs delivered by Group-F and the exact integration points available to Group-E for MPI-based distributed extension (Subtask-07 and beyond).

Group-F's implementation is complete. All APIs listed below are frozen. Group-E should not modify any Group-F source files. Only the files explicitly marked as swappable should be replaced.

---

## Repository Structure (Integration-Relevant)

```text
include/
    shared_buffer.h       ← Primary API — include in all MPI modules
    producer.h            ← ProducerArgs, producer_thread()
    packet_generator.h    ← generate_packet() — SWAPPABLE
    consumer.h            ← ConsumerArgs, consumer_thread(), producers_done
    packet_processor.h    ← process_packet() — SWAPPABLE
    media_player.h        ← Orchestration API

src/
    buffer/shared_buffer.c        ← Frozen
    producer/producer.c           ← Frozen
    producer/packet_generator.c   ← SWAPPABLE (data source)
    consumer/consumer.c           ← Frozen
    consumer/packet_processor.c   ← SWAPPABLE (processing logic)
    integration/media_player.c    ← Frozen
    integration/main_media_player.c ← Entry point (can be adapted)
```

---

## Frozen APIs

### 1. SharedBuffer API (`shared_buffer.h`)

```c
int  buffer_init(SharedBuffer *buf);
int  enqueue(SharedBuffer *buf, const DataUnit *packet);
int  dequeue(SharedBuffer *buf, DataUnit *out);
void buffer_destroy(SharedBuffer *buf);
```

**Return codes:**
```c
#define BUF_OK     0
#define BUF_FULL  -1
#define BUF_EMPTY -2
#define BUF_ERR   -3
```

**Rules:**
- Never access `head`, `tail`, `count`, or `slots[]` directly.
- Always use `enqueue()` and `dequeue()`.
- `buffer_init()` must be called before any threads are spawned.
- `buffer_destroy()` must be called after all threads have joined.

---

### 2. DataUnit Structure (`shared_buffer.h`)

```c
typedef struct {
    int    packet_id;        /* Unique packet identifier      */
    char   source[32];       /* Origin site or node name      */
    char   data[1024];       /* Payload                       */
    int    size;             /* Valid bytes in data[]         */
    time_t timestamp;        /* Packet creation time          */
} DataUnit;
```

This structure is the unit of transfer across the shared buffer. MPI nodes should populate `DataUnit` instances and pass them to `enqueue()`.

---

### 3. ProducerArgs Structure (`producer.h`)

```c
typedef struct {
    SharedBuffer *buffer;
    char          source_name[32];
    int           packets_to_generate;
    int           start_packet_id;
} ProducerArgs;
```

`producer_thread()` signature:
```c
void *producer_thread(void *arg);  /* arg = ProducerArgs* */
```

**Notes:**
- `start_packet_id` ensures globally unique packet IDs across multiple producer threads.
- Each producer increments from `start_packet_id` to `start_packet_id + packets_to_generate - 1`.

---

### 4. ConsumerArgs Structure (`consumer.h`)

```c
typedef struct {
    SharedBuffer *buffer;
    int           consumer_id;
} ConsumerArgs;

extern volatile int producers_done;  /* Set to 1 by integration layer after all producers join */
```

`consumer_thread()` signature:
```c
void *consumer_thread(void *arg);  /* arg = ConsumerArgs* */
```

**Shutdown mechanism:**
- `consumer_thread()` exits when `producers_done == 1` AND `dequeue()` returns `BUF_EMPTY`.
- The integration layer sets `producers_done = 1` after all producer threads have joined.

---

### 5. Integration Orchestration API (`media_player.h`)

```c
typedef struct {
    int num_producers;
    int num_consumers;
    int packets_per_producer;
} MediaPlayerConfig;

int  media_player_init(MediaPlayerContext *ctx, const MediaPlayerConfig *cfg);
int  media_player_run(MediaPlayerContext *ctx, const MediaPlayerConfig *cfg);
void media_player_destroy(MediaPlayerContext *ctx);
void media_player_print_summary(const MediaPlayerConfig *cfg, int packets_consumed);
```

---

## Swappable Components

### packet_generator.c — Data Source

**Current implementation:** Generates simulated telecom packets with synthetic payloads.

**Interface (must be preserved):**
```c
DataUnit generate_packet(int packet_id, const char *source);
```

**Group-E replacement:** Replace `packet_generator.c` with an implementation that receives real data from MPI nodes. The function must return a fully populated `DataUnit` by value. `producer.c` does not need to change.

---

### packet_processor.c — Processing Logic

**Current implementation:** Prints packet metadata and payload to stdout.

**Interface (must be preserved):**
```c
void process_packet(const DataUnit *packet, int consumer_id);
```

**Group-E replacement:** Replace `packet_processor.c` with an implementation that performs telecom analytics, MPI transmission, database insertion, or packet inspection. `consumer.c` does not need to change.

---

## Integration Sequence

```text
1. buffer_init(&buffer)

2. producers_done = 0

3. pthread_create → consumer_thread() × N
      ConsumerArgs: { &buffer, consumer_id }

4. pthread_create → producer_thread() × M
      ProducerArgs: { &buffer, source_name, packets_to_generate, start_packet_id }

5. pthread_join all producer threads

6. producers_done = 1
   pthread_cond_broadcast (wake consumers)

7. pthread_join all consumer threads

8. buffer_destroy(&buffer)
```

---

## Subtask-07 Compatibility Notes

- `dequeue()` uses `pthread_rwlock_wrlock` — safe for multiple concurrent consumers.
- `pthread_rwlock_rdlock` is intentionally reserved for a future `peek()` operation if Subtask-07 requires read-only buffer inspection without removal.
- `SharedBuffer.count` is accessible directly for status queries after all threads have exited (post-join), but must not be read concurrently during execution.

---

## Build Reference

```bash
# Full build
make all

# Run integration test suite
make run-integration

# Run standalone binary
make run-media-player NUM=4 CON=4 PKT=20
```

Manual compilation with all modules:
```bash
gcc -Wall -Wextra -O2 -pthread -Iinclude \
    src/integration/media_player.c \
    src/integration/main_media_player.c \
    src/buffer/shared_buffer.c \
    src/producer/producer.c \
    src/producer/packet_generator.c \
    src/consumer/consumer.c \
    src/consumer/packet_processor.c \
    -o build/media_player
```

---

## Validated Test Results

| Test                                      | Status |
| ----------------------------------------- | ------ |
| Shared Buffer Smoke Test (7 cases)        | PASS   |
| Producer Test (4P, 20 pkts each)          | PASS   |
| Consumer Test (4C, 20 pkts)               | PASS   |
| Integration — Basic Pipeline (1P+1C)      | PASS   |
| Integration — Standard Pipeline (4P+4C)   | PASS   |
| Integration — Producer-Heavy (6P+2C)      | PASS   |
| Integration — Consumer-Heavy (2P+6C)      | PASS   |

---

## Contact

For questions about the shared buffer, producer, consumer, or integration layer — contact Bhargav (Group-F Lead).