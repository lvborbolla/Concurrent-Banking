# Concurrent Banking System

A multi-threaded banking database simulator built using POSIX threads (`pthreads`) that demonstrates concurrency control, reader-writer synchronization, bounded buffer management using semaphores, and deadlock prevention through lock ordering.

---

# Group Members

- Luis Victor Borbolla
- Eryl Josep Aspera

---

# Features

## Implemented Features

- Multi-threaded transaction execution using `pthread_create`
- Global timer thread with simulated clock ticks
- Reader-writer synchronization using `pthread_rwlock_t`
- Deadlock prevention using lock ordering
- Concurrent banking operations:
  - DEPOSIT
  - WITHDRAW
  - TRANSFER
  - BALANCE
- Bounded buffer pool implementation using semaphores
- Transaction scheduling using start ticks
- Thread-safe logging system
- Transaction performance metrics
- Buffer pool statistics
- Account loading from file
- Transaction trace parsing
- Automated test execution using `make test`
- ThreadSanitizer debug build support

---

# Project Structure

```text
bankdb/
├── Makefile
├── README.md
├── docs/
│   └── design.md
├── include/
├── src/
└── tests/
```

---

# Compilation Instructions

## Normal Build

```bash
make all
```

This compiles the project using:

```text
-pthread -O2 -Wall -Wextra
```

---

## Debug Build (ThreadSanitizer)

```bash
make debug
```

This enables:

```text
-g -fsanitize=thread -pthread
```

Used for race condition detection.

---

# Running the Program

## Basic Usage

```bash
./bankdb --accounts=tests/accounts.txt \
         --trace=tests/trace_simple.txt \
         --deadlock=prevention
```

---

# Command-Line Options

| Option | Description |
|---|---|
| `--accounts=FILE` | Initial account balances |
| `--trace=FILE` | Transaction trace file |
| `--deadlock=prevention` | Use lock-order deadlock prevention |
| `--tick-ms=N` | Tick interval in milliseconds |
| `--verbose` | Enable verbose logging |

---

# Example Commands

## Simple Sequential Test

```bash
./bankdb --accounts=tests/accounts.txt \
         --trace=tests/trace_simple.txt
```

---

## Concurrent Readers Test

```bash
./bankdb --accounts=tests/accounts.txt \
         --trace=tests/trace_readers.txt
```

---

## Deadlock Prevention Test

```bash
./bankdb --accounts=tests/accounts.txt \
         --trace=tests/trace_deadlock.txt
```

---

## Buffer Pool Saturation Test

```bash
./bankdb --accounts=tests/accounts.txt \
         --trace=tests/trace_buffer.txt
```

---

# Automated Testing

Run all provided test cases:

```bash
make test
```

This executes:

1. trace_simple.txt
2. trace_readers.txt
3. trace_deadlock.txt
4. trace_abort.txt
5. trace_buffer.txt

---

# Cleaning Build Files

```bash
make clean
```

Removes:

- executable binaries
- object files

---

# ThreadSanitizer Testing

Build:

```bash
make debug
```

Run:

```bash
./bankdb --accounts=tests/accounts.txt \
         --trace=tests/trace_readers.txt
```

Expected result:

```text
No ThreadSanitizer warnings
```

---

# Supported Transaction Operations

## DEPOSIT

```text
DEPOSIT account_id amount
```

Adds money to an account.

---

## WITHDRAW

```text
WITHDRAW account_id amount
```

Removes money from an account.

Transaction aborts if insufficient funds exist.

---

## TRANSFER

```text
TRANSFER from_account to_account amount
```

Transfers money safely using ordered locking.

---

## BALANCE

```text
BALANCE account_id
```

Reads account balance concurrently using reader locks.

---

# Known Limitations

- Deadlock detection strategy is not implemented
- Only deadlock prevention via lock ordering is supported
- No persistent storage (in-memory only)
- Buffer pool does not implement LRU replacement
- Account IDs are limited to `MAX_ACCOUNTS`
- No rollback recovery system for partial transaction failures
- No transaction priorities or starvation prevention

---

# Concurrency Mechanisms Used

| Mechanism | Purpose |
|---|---|
| `pthread_create` | Concurrent transaction threads |
| `pthread_rwlock_t` | Reader-writer synchronization |
| `pthread_mutex_t` | Shared metadata protection |
| `sem_t` | Bounded buffer synchronization |
| `pthread_cond_t` | Tick synchronization |
| Thread-local storage | Per-thread transaction tracking |

---

# Output Metrics

The system reports:

- Transaction completion status
- Wait ticks
- Throughput
- Buffer pool statistics
- Peak buffer usage
- Blocked operations
- Final account balances

---

# Course Concepts Demonstrated

- Producer-consumer problem
- Deadlock prevention
- Reader-writer synchronization
- Concurrent transaction scheduling
- Shared resource protection
- Lock contention
- Thread synchronization
- Concurrency performance analysis