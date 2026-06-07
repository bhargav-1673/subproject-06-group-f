# Shared Buffer API

## DataUnit

- packet_id
- source
- data
- size
- timestamp

## SharedBuffer

Circular buffer protected by pthread_rwlock_t.

## Functions

- buffer_init()
- enqueue()
- dequeue()
- buffer_destroy()