#ifndef METRICS_H
#define METRICS_H

#include <pthread.h>
#include <time.h>

#define MAX_THREADS 16

typedef struct MetricsTracker {
    pthread_mutex_t lock;
    
    struct timespec start_time;
    struct timespec end_time;
    
    int packets_generated;
    int packets_enqueued;
    int packets_dequeued;
    int packets_dropped;
    
    long long total_latency_ms;  /* cumulative latency in milliseconds */
    
    long long occupancy_sum;     /* cumulative queue occupancy counts */
    int occupancy_samples;
    int max_occupancy;

    /* Per-thread stats (0-based index) */
    int producer_enqueued[MAX_THREADS];
    int producer_dropped[MAX_THREADS];
    int consumer_processed[MAX_THREADS];
} MetricsTracker;

void metrics_init(MetricsTracker *tracker);
void metrics_start(MetricsTracker *tracker);
void metrics_stop(MetricsTracker *tracker);

/* Aggregate variants — used by buffer/producer standalone tests */
void metrics_log_enqueue(MetricsTracker *tracker, int count);
void metrics_log_dequeue(MetricsTracker *tracker, int count, time_t packet_timestamp);
void metrics_log_drop(MetricsTracker *tracker);

/* Per-thread variants — used by integration layer */
void metrics_log_enqueue_by(MetricsTracker *tracker, int count, int producer_id);
void metrics_log_drop_by(MetricsTracker *tracker, int producer_id);
void metrics_log_dequeue_by(MetricsTracker *tracker, int count, time_t packet_timestamp, int consumer_id);

void metrics_print_report(const MetricsTracker *tracker, int num_producers, int num_consumers, int expected_packets);

/* Thread-safe console logger */
void safe_log(const char *format, ...);

#endif /* METRICS_H */