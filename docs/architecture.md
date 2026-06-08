# System Architecture

## Overview

The Multi-Threaded Telecom Data Extraction Server simulates telecom packet flow from multiple city sites to a centralized processing server.

## Architecture

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
