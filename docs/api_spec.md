# Shared Buffer API Specification

## DataUnit

typedef struct
{
int packet_id;
char source[32];
char data[1024];
int size;
time_t timestamp;
} DataUnit;

## SharedBuffer

typedef struct
{
DataUnit slots[BUFFER_SIZE];

```
int head;
int tail;
int count;

pthread_rwlock_t lock;
```

} SharedBuffer;

## Public Functions

void buffer_init(SharedBuffer *sb);

void buffer_destroy(SharedBuffer *sb);

int enqueue(
SharedBuffer *sb,
const DataUnit *packet);

int dequeue(
SharedBuffer *sb,
DataUnit *packet);

## Return Codes

BUF_OK      = 0

BUF_FULL    = -1

BUF_EMPTY   = -2

BUF_ERR     = -3

## Integration Rules

Consumers and Producers must never access:

head

tail

count

slots[]

directly.

Use only:

enqueue()

dequeue()

---

# Producer API Specification

## ProducerArgs

typedef struct
{
SharedBuffer *buffer;
char source_name[32];
int packets_to_generate;
int start_packet_id;
} ProducerArgs;

## Public Functions

void *producer_thread(void *arg);

DataUnit generate_packet(
int packet_id,
const char *source);

## Notes

producer_thread() takes a ProducerArgs* and generates packets_to_generate packets starting at start_packet_id.

start_packet_id guarantees globally unique packet IDs across multiple producer threads.

generate_packet() lives in packet_generator.c — swappable for real telecom data sources without modifying producer.c.

producer_thread() calls enqueue() only. It never accesses buffer internals directly.

---

# Consumer API Specification

## ConsumerArgs

typedef struct
{
SharedBuffer *buffer;
int consumer_id;
} ConsumerArgs;

## Global Shutdown Flag

extern volatile int producers_done;

Set to 1 by the integration layer after all producer threads have been joined.

## Public Functions

void *consumer_thread(void *arg);

void process_packet(
const DataUnit *packet,
int consumer_id);

## Shutdown Behavior

consumer_thread() loops on dequeue().

On BUF_EMPTY, it checks producers_done:

If producers_done == 1, the consumer exits.

If producers_done == 0, the consumer sleeps briefly and retries.

process_packet() lives in packet_processor.c — swappable for analytics, MPI transmission, or database insertion without modifying consumer.c.

---

# Integration API Specification

## MediaPlayerConfig

typedef struct
{
int num_producers;
int num_consumers;
int packets_per_producer;
} MediaPlayerConfig;

## MediaPlayerContext

typedef struct
{
SharedBuffer buffer;

pthread_cond_t  producers_done_cond;
pthread_mutex_t producers_done_mutex;
int             producers_done;

pthread_t *producer_threads;
pthread_t *consumer_threads;
} MediaPlayerContext;

## Public Functions

int media_player_init(
MediaPlayerContext *ctx,
const MediaPlayerConfig *cfg);

int media_player_run(
MediaPlayerContext *ctx,
const MediaPlayerConfig *cfg);

void media_player_destroy(
MediaPlayerContext *ctx);

void media_player_print_summary(
const MediaPlayerConfig *cfg,
int packets_consumed);

## Notes

media_player_run() spawns consumer threads before producer threads, joins all producers, then broadcasts the producer-done signal to consumers via pthread_cond_broadcast().

The cond var and mutex in MediaPlayerContext exist for hyPACK assignment compliance. The actual shutdown check in consumer_thread() reads the producers_done global directly — both are set together under the same mutex lock.