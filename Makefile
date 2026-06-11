CC     = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -Iinclude

BUILD_DIR = build

BUFFER_TEST   = $(BUILD_DIR)/test_buffer
PRODUCER_TEST = $(BUILD_DIR)/test_producer
CONSUMER_TEST = $(BUILD_DIR)/test_consumer

# Default CLI args for run-producer (override on command line)
# Usage: make run-producer NUM=4 PKT=10
NUM ?= 4
PKT ?= 20

all: buffer_test producer_test consumer_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# --------------------------------------------------
# Shared Buffer Test
# --------------------------------------------------

buffer_test: $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		tests/test_buffer.c \
		src/buffer/shared_buffer.c \
		-o $(BUFFER_TEST)

# --------------------------------------------------
# Producer Test
# --------------------------------------------------

producer_test: $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		tests/test_producer.c \
		src/buffer/shared_buffer.c \
		src/producer/producer.c \
		src/producer/packet_generator.c \
		-o $(PRODUCER_TEST)

# --------------------------------------------------
# Consumer Test
# --------------------------------------------------

consumer_test: $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		tests/test_consumer.c \
		src/buffer/shared_buffer.c \
		src/consumer/consumer.c \
		src/consumer/packet_processor.c \
		-o $(CONSUMER_TEST)

# --------------------------------------------------
# Run Targets
# --------------------------------------------------

run-buffer: buffer_test
	./$(BUFFER_TEST)

# Single producer by default — override with: make run-producer NUM=4 PKT=10
run-producer: producer_test
	./$(PRODUCER_TEST) $(NUM) $(PKT)

run-consumer: consumer_test
	./$(CONSUMER_TEST) $(NUM) $(PKT)

# --------------------------------------------------
# Run All Tests
# --------------------------------------------------

run-all: run-buffer run-producer run-consumer

# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all \
	buffer_test \
	producer_test \
	run-buffer \
	run-producer \
	run-consumer \
	run-all \
	clean