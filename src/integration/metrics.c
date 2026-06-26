#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <time.h>

#include "metrics.h"

/* Static logging mutex to serialize stdout prints */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void metrics_init(MetricsTracker *tracker) {
    if (tracker == NULL) return;
    
    pthread_mutex_init(&tracker->lock, NULL);
    tracker->packets_generated = 0;
    tracker->packets_enqueued = 0;
    tracker->packets_dequeued = 0;
    tracker->packets_dropped = 0;
    tracker->total_latency_ms = 0;
    tracker->occupancy_sum = 0;
    tracker->occupancy_samples = 0;
    tracker->max_occupancy = 0;
    tracker->start_time.tv_sec = 0;
    tracker->start_time.tv_nsec = 0;
    tracker->end_time.tv_sec = 0;
    tracker->end_time.tv_nsec = 0;

    for (int i = 0; i < MAX_THREADS; i++) {
        tracker->producer_enqueued[i] = 0;
        tracker->producer_dropped[i]  = 0;
        tracker->consumer_processed[i] = 0;
    }
}

void metrics_start(MetricsTracker *tracker) {
    if (tracker == NULL) return;
    pthread_mutex_lock(&tracker->lock);
    clock_gettime(CLOCK_MONOTONIC, &tracker->start_time);
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_stop(MetricsTracker *tracker) {
    if (tracker == NULL) return;
    pthread_mutex_lock(&tracker->lock);
    clock_gettime(CLOCK_MONOTONIC, &tracker->end_time);
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_log_enqueue(MetricsTracker *tracker, int count) {
    if (tracker == NULL) return;
    pthread_mutex_lock(&tracker->lock);
    
    tracker->packets_generated++;
    tracker->packets_enqueued++;
    
    tracker->occupancy_sum += count;
    tracker->occupancy_samples++;
    if (count > tracker->max_occupancy) {
        tracker->max_occupancy = count;
    }
    
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_log_drop(MetricsTracker *tracker) {
    if (tracker == NULL) return;
    pthread_mutex_lock(&tracker->lock);
    
    tracker->packets_generated++;
    tracker->packets_dropped++;
    
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_log_dequeue(MetricsTracker *tracker, int count, time_t packet_timestamp) {
    if (tracker == NULL) return;
    
    /* Get current millisecond epoch time */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long long current_ms = (long long)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000);
    long long latency = current_ms - (long long)packet_timestamp;
    if (latency < 0) {
        latency = 0; /* Guard against clock resolution/rounding drifts */
    }
    
    pthread_mutex_lock(&tracker->lock);
    
    tracker->packets_dequeued++;
    tracker->total_latency_ms += latency;
    
    tracker->occupancy_sum += count;
    tracker->occupancy_samples++;
    if (count > tracker->max_occupancy) {
        tracker->max_occupancy = count;
    }
    
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_log_enqueue_by(MetricsTracker *tracker, int count, int producer_id) {
    if (tracker == NULL || producer_id < 0 || producer_id >= MAX_THREADS) return;
    pthread_mutex_lock(&tracker->lock);
    tracker->packets_generated++;
    tracker->packets_enqueued++;
    tracker->producer_enqueued[producer_id]++;
    tracker->occupancy_sum += count;
    tracker->occupancy_samples++;
    if (count > tracker->max_occupancy) tracker->max_occupancy = count;
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_log_drop_by(MetricsTracker *tracker, int producer_id) {
    if (tracker == NULL || producer_id < 0 || producer_id >= MAX_THREADS) return;
    pthread_mutex_lock(&tracker->lock);
    tracker->packets_generated++;
    tracker->packets_dropped++;
    tracker->producer_dropped[producer_id]++;
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_log_dequeue_by(MetricsTracker *tracker, int count, time_t packet_timestamp, int consumer_id) {
    if (tracker == NULL || consumer_id < 0 || consumer_id >= MAX_THREADS) return;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long long current_ms = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    long long latency = current_ms - (long long)packet_timestamp;
    if (latency < 0) latency = 0;
    pthread_mutex_lock(&tracker->lock);
    tracker->packets_dequeued++;
    tracker->total_latency_ms += latency;
    tracker->consumer_processed[consumer_id]++;
    tracker->occupancy_sum += count;
    tracker->occupancy_samples++;
    if (count > tracker->max_occupancy) tracker->max_occupancy = count;
    pthread_mutex_unlock(&tracker->lock);
}

void metrics_print_report(const MetricsTracker *tracker, int num_producers, int num_consumers, int expected_packets) {
    if (tracker == NULL) return;
    
    double elapsed_sec = (tracker->end_time.tv_sec - tracker->start_time.tv_sec) +
                         (tracker->end_time.tv_nsec - tracker->start_time.tv_nsec) / 1000000000.0;
                         
    if (elapsed_sec <= 0.0) {
        elapsed_sec = 0.0001; /* Prevent division by zero */
    }
    
    double throughput = tracker->packets_dequeued / elapsed_sec;
    double avg_latency = 0.0;
    if (tracker->packets_dequeued > 0) {
        avg_latency = (double)tracker->total_latency_ms / tracker->packets_dequeued;
    }
    
    double avg_occupancy = 0.0;
    if (tracker->occupancy_samples > 0) {
        avg_occupancy = (double)tracker->occupancy_sum / tracker->occupancy_samples;
    }
    
    double loss_rate = 0.0;
    if (tracker->packets_generated > 0) {
        loss_rate = ((double)tracker->packets_dropped / tracker->packets_generated) * 100.0;
    }
    
    pthread_mutex_lock(&log_mutex);
    printf("\n");
    printf("==================================================\n");
    printf("         TELEMETRY PERFORMANCE REPORT             \n");
    printf("==================================================\n");
    printf(" Runtime Configuration:\n");
    printf("   Producers Spawned       : %d\n", num_producers);
    printf("   Consumers Spawned       : %d\n", num_consumers);
    printf("   Expected Packets        : %d\n", expected_packets);
    printf("\n");
    printf(" Traffic Statistics:\n");
    printf("   Total Generated Packets : %d\n", tracker->packets_generated);
    printf("   Successfully Enqueued   : %d\n", tracker->packets_enqueued);
    printf("   Successfully Dequeued   : %d\n", tracker->packets_dequeued);
    printf("   Dropped Packets (Full)  : %d\n", tracker->packets_dropped);
    printf("   Packet Loss Rate        : %.2f %%\n", loss_rate);
    printf("\n");
    printf(" Performance Metrics:\n");
    printf("   Elapsed Execution Time  : %.6f seconds\n", elapsed_sec);
    printf("   Sustained Throughput    : %.2f packets/sec\n", throughput);
    printf("   Average Latency         : %.2f ms\n", avg_latency);
    printf("\n");
    printf(" Queue Occupancy:\n");
    printf("   Average Buffer Load     : %.2f slots\n", avg_occupancy);
    printf("   Peak Buffer Load        : %d slots\n", tracker->max_occupancy);
    printf("\n");
    printf(" Producer Breakdown:\n");
    for (int i = 0; i < num_producers && i < MAX_THREADS; i++) {
        printf("   Producer-%-2d             : %d enqueued, %d dropped\n",
               i + 1,
               tracker->producer_enqueued[i],
               tracker->producer_dropped[i]);
    }
    printf("\n");
    printf(" Consumer Breakdown:\n");
    for (int i = 0; i < num_consumers && i < MAX_THREADS; i++) {
        printf("   Consumer-%-2d             : %d packets processed\n",
               i + 1,
               tracker->consumer_processed[i]);
    }
    printf("==================================================\n\n");
    pthread_mutex_unlock(&log_mutex);
}

void safe_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&log_mutex);
    
    va_end(args);
}