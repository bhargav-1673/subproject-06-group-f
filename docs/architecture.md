# System Architecture

## Overview

The Multi-Threaded Telecom Data Extraction Server simulates telecom packet flow from multiple city sites to a centralized processing server.

## Architecture

```text
Producer Threads
|
v
+---------------------+
|   Shared Buffer     |
|   Circular Queue    |
+---------------------+
|
v
Consumer Threads
```
## Components

### Producer Layer

Responsible for generating DataUnit packets from telecom sites.

Examples:

* Hyderabad
* Mumbai
* Delhi
* Bangalore

Producer threads communicate only through the public buffer API.

### Shared Buffer Layer

Thread-safe circular queue.

Responsibilities:

* Store DataUnit packets
* Handle concurrent access
* Detect BUFFER_FULL
* Detect BUFFER_EMPTY

Synchronization:

* pthread_rwlock_wrlock()
* pthread_rwlock_unlock()

### Consumer Layer

Responsible for extracting packets and processing them.

Examples:

* Logging
* Analytics
* Telecom packet inspection

### Integration Layer

Responsible for:

* Thread creation
* Thread coordination
* Buffer initialization
* Buffer destruction
* Producer-done signaling to consumers

Implementation: media_player.c, main_media_player.c, media_player.h

The integration layer spawns consumer threads first, then producer threads. After all producer threads are joined, it sets the producers_done flag and broadcasts a condition variable to wake any sleeping consumer threads. It then joins all consumer threads and destroys the buffer.

## Shutdown Sequence

```text
Main Thread
     |
     v
buffer_init()
     |
     v
Spawn Consumer Threads  (wait on dequeue/cond)
     |
     v
Spawn Producer Threads  (enqueue packets)
     |
     v
pthread_join() all Producers
     |
     v
Set producers_done = 1
pthread_cond_broadcast()
     |
     v
Consumers drain remaining packets, then exit
     |
     v
pthread_join() all Consumers
     |
     v
buffer_destroy()
```

## Synchronization Summary

| Layer       | Mechanism                          | Purpose                          |
|-------------|-------------------------------------|-----------------------------------|
| Buffer      | pthread_rwlock_t                    | Protect head/tail/count/slots[]   |
| Integration | pthread_cond_t + pthread_mutex_t    | Producer-done signal to consumers |

## Telecom Scenario Mapping

| Real-World Role     | Implementation                  |
|----------------------|----------------------------------|
| Telecom Site         | Producer thread (source_name)    |
| Packet Transmission  | enqueue()                        |
| Central Server Queue | SharedBuffer (circular)          |
| Data Extraction      | Consumer thread + process_packet() |