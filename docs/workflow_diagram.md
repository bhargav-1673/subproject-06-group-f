# Workflow Diagram

## End-to-End Pipeline Flow

```text
                         ┌─────────────────────┐
                         │   main_media_player  │
                         │   (or test runner)   │
                         └──────────┬───────────┘
                                    │
                                    v
                         ┌─────────────────────┐
                         │  media_player_init() │
                         │  buffer_init()        │
                         │  cond/mutex init      │
                         └──────────┬───────────┘
                                    │
                                    v
                         ┌─────────────────────┐
                         │  media_player_run()  │
                         └──────────┬───────────┘
                                    │
                ┌───────────────────┴───────────────────┐
                v                                         v
   ┌─────────────────────────┐               ┌─────────────────────────┐
   │  Consumer Thread(s)      │               │  Producer Thread(s)      │
   │  (spawned first)         │               │  (spawned second)        │
   └────────────┬─────────────┘               └────────────┬─────────────┘
                │                                            │
                │                                            v
                │                                  generate_packet()
                │                                            │
                │                                            v
                │                                       enqueue()
                │                                            │
                │                                            v
                │                                ┌─────────────────────┐
                │<───────────────────────────────│   Shared Buffer      │
                │                                │   (circular queue)   │
                │                                │   pthread_rwlock_t   │
                │                                └─────────────────────┘
                v
           dequeue()
                │
                v
        process_packet()
                │
                v
      BUF_EMPTY? ──No──> loop back to dequeue()
                │
               Yes
                │
                v
      producers_done == 1? ──No──> nanosleep(250ms) ──> loop back to dequeue()
                │
               Yes
                │
                v
         Consumer Exits
```

## Producer Lifecycle

```text
producer_thread(ProducerArgs*)
        |
        v
for i in [0, packets_to_generate):
        |
        v
   generate_packet(start_packet_id + i, source_name)
        |
        v
   enqueue(buffer, &packet)
        |
        +--> BUF_OK     → log "Packet enqueued"
        +--> BUF_FULL   → log "Packet dropped"
        +--> BUF_ERR    → log "ERROR on enqueue"
        |
        v
   nanosleep(100ms)
        |
        v
   (loop or exit after last packet)
        |
        v
   pthread_exit(NULL)
```

## Consumer Lifecycle

```text
consumer_thread(ConsumerArgs*)
        |
        v
   loop:
        |
        v
   dequeue(buffer, &packet)
        |
        +--> BUF_OK     → process_packet(&packet, consumer_id) → loop
        |
        +--> BUF_EMPTY  → producers_done == 1?
        |                       |
        |                      Yes → exit thread
        |                       |
        |                      No  → nanosleep(250ms) → loop
        |
        +--> BUF_ERR    → log error → exit thread
```

## Integration Shutdown Signal Flow

```text
Main Thread                          Consumer Threads
    |                                       |
    | pthread_join() all producers          |
    |<--------------------------------------|  (consumers polling dequeue/sleep)
    |                                       |
    | lock(producers_done_mutex)            |
    | producers_done = 1   (global)         |
    | pthread_cond_broadcast()              |
    | unlock(producers_done_mutex)          |
    |                                       |
    |--------------------------------------->|  next BUF_EMPTY check sees flag=1
    |                                       |  consumer exits cleanly
    | pthread_join() all consumers          |
    |                                       |
    v
buffer_destroy()
```

## Telecom Data Flow

```text
Site-Hyderabad  ─┐
Site-Delhi      ─┤
Site-Mumbai     ─┼──> Producer Threads ──> enqueue() ──> Shared Circular Buffer
Site-Chennai    ─┤                                              │
Site-Bangalore  ─┘                                              v
                                                          Consumer Threads
                                                                  │
                                                                  v
                                                          process_packet()
                                                          (stdout / future:
                                                           analytics, MPI,
                                                           DB insertion)
```