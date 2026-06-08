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
