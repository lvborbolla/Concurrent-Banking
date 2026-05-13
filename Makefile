CC = gcc

CFLAGS = -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -Wall -Wextra -g -fsanitize=thread -pthread

INCLUDES = -Iinclude

SRC = src/main.c \
      src/bank.c \
      src/transaction.c \
      src/timer.c \
      src/lock_mgr.c \
      src/buffer_pool.c \
      src/metrics.c \
      src/utils.c

TARGET = bankdb

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET)

debug:
	$(CC) $(DEBUG_FLAGS) $(INCLUDES) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f src/*.o

test: all
	@echo "===== TEST 1: SIMPLE (no accounts file) ====="
	./bankdb --verbose

	@echo "===== TEST 2: READERS (with accounts) ====="
	./bankdb --accounts=tests/accounts.txt --trace=tests/trace_readers.txt --verbose

	@echo "===== TEST 3: DEADLOCK (with accounts) ====="
	./bankdb --accounts=tests/accounts.txt --trace=tests/trace_deadlock.txt --verbose

	@echo "===== TEST 4: ABORT (no accounts file) ====="
	./bankdb --trace=tests/trace_abort.txt --verbose

	@echo "===== TEST 5: BUFFER (with accounts) ====="
	./bankdb --accounts=tests/accounts.txt --trace=tests/trace_buffer.txt --verbose