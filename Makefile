CC = gcc

CFLAGS = -Wall -Wextra -O2 -pthread -Iinclude

BUILD_DIR = build

BUFFER_TEST = $(BUILD_DIR)/test_buffer
PRODUCER_TEST = $(BUILD_DIR)/test_producer
MULTI_PRODUCER_TEST = $(BUILD_DIR)/test_multiple_producers

all: buffer_test producer_test multi_producer_test

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
	-o $(PRODUCER_TEST)

# --------------------------------------------------
# Multi Producer Test
# --------------------------------------------------

multi_producer_test: $(BUILD_DIR)
	$(CC) $(CFLAGS) \
	tests/test_multiple_producers.c \
	src/buffer/shared_buffer.c \
	src/producer/producer.c \
	-o $(MULTI_PRODUCER_TEST)

# --------------------------------------------------
# Run Targets
# --------------------------------------------------

run-buffer: buffer_test
	./$(BUFFER_TEST)

run-producer: producer_test
	./$(PRODUCER_TEST)

run-multi: multi_producer_test
	./$(MULTI_PRODUCER_TEST)

# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all \
	buffer_test \
	producer_test \
	multi_producer_test \
	run-buffer \
	run-producer \
	run-multi \
	clean