CC     = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -Iinclude

BUILD_DIR = build

BUFFER_TEST      = $(BUILD_DIR)/test_buffer
PRODUCER_TEST    = $(BUILD_DIR)/test_producer
CONSUMER_TEST    = $(BUILD_DIR)/test_consumer
INTEGRATION_TEST = $(BUILD_DIR)/test_integration
MEDIA_PLAYER     = $(BUILD_DIR)/media_player

# Default CLI args (override on command line)
# Usage: make run-producer NUM=4 PKT=10
# Usage: make run-consumer NUM=4 PKT=20
# Usage: make run-media-player NUM=4 CON=4 PKT=20
NUM ?= 4
PKT ?= 20
CON ?= 4

all: buffer_test producer_test consumer_test integration_test media_player

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
		src/integration/metrics.c \
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
		src/integration/metrics.c \
		-o $(CONSUMER_TEST)

# --------------------------------------------------
# Integration Test
# --------------------------------------------------

integration_test: $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		tests/test_integration.c \
		src/integration/media_player.c \
		src/integration/metrics.c \
		src/buffer/shared_buffer.c \
		src/producer/producer.c \
		src/producer/packet_generator.c \
		src/consumer/consumer.c \
		src/consumer/packet_processor.c \
		-o $(INTEGRATION_TEST)

# --------------------------------------------------
# Media Player (standalone binary)
# --------------------------------------------------

media_player: $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		src/integration/media_player.c \
		src/integration/metrics.c \
		src/integration/main_media_player.c \
		src/buffer/shared_buffer.c \
		src/producer/producer.c \
		src/producer/packet_generator.c \
		src/consumer/consumer.c \
		src/consumer/packet_processor.c \
		-o $(MEDIA_PLAYER)

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

# Runs all 4 integration test cases automatically
run-integration: integration_test
	./$(INTEGRATION_TEST)

# Manual integration run — override with: make run-media-player NUM=4 CON=4 PKT=20
run-media-player: media_player
	./$(MEDIA_PLAYER) $(NUM) $(CON) $(PKT)

# --------------------------------------------------
# Run All Tests
# --------------------------------------------------

run-all: run-buffer run-producer run-consumer run-integration

# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all \
	buffer_test \
	producer_test \
	consumer_test \
	integration_test \
	media_player \
	run-buffer \
	run-producer \
	run-consumer \
	run-integration \
	run-media-player \
	run-all \
	clean