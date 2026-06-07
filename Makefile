CC=gcc

CFLAGS=-Wall -Wextra -O2 -pthread -Iinclude

TARGET=build/test_buffer

all: buffer_test

buffer_test:
	mkdir -p build
	$(CC) $(CFLAGS) tests/test_buffer.c src/buffer/shared_buffer.c -o $(TARGET)

run: buffer_test
	./$(TARGET)

clean:
	rm -rf build

.PHONY: all buffer_test run clean
