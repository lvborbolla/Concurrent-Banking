

## Concurrent Banking System Design Documentation
CMSC 125 - Lab 3 Design notes for Concurrent Banking - an in-memory bank that processes concurrent transactions, implements proper isolation through locking, manages a bounded buffer pool with semaphores, and handles deadlock through either prevention or detection

By Luis Borbolla & Eryl Joseph Aspera

### Project Overview

This project implements a concurrent banking database system using POSIX threads (`pthreads`) and synchronization primitives. The system supports concurrent transaction execution while preserving correctness, isolation, and consistency through reader-writer locks, semaphores, and deadlock handling.

The implementation includes:

* Multi-threaded transaction execution
* Reader-writer synchronization
* Deadlock prevention
* Bounded buffer pool using semaphores
* Global timer thread for concurrency simulation
* Transaction performance metrics
* Thread-safe account management

---

# 1. Deadlock Strategy Choice

## Chosen Strategy: Deadlock Prevention via Lock Ordering

The system uses **deadlock prevention** instead of deadlock detection.

### Why We Chose Prevention

We selected prevention because:

* It is simpler and safer to implement correctly
* It introduces lower runtime overhead
* It avoids continuously scanning wait-for graphs
* It guarantees deadlock freedom before deadlock can occur
* It works naturally for banking transfers involving two account locks

Since transfers only require locking two accounts, global lock ordering provides a deterministic and efficient solution.

---

## Lock Ordering Rule

For every transfer:

```c
int first = (from_id < to_id) ? from_id : to_id;
int second = (from_id < to_id) ? to_id : from_id;
```

Locks are always acquired in ascending account ID order:

```c
pthread_rwlock_wrlock(&bank.accounts[first].lock);
pthread_rwlock_wrlock(&bank.accounts[second].lock);
```

This ordering is applied consistently across all transactions.

---

## Proof That Circular Wait Is Eliminated

Deadlock requires all four Coffman conditions:

1. Mutual Exclusion
2. Hold and Wait
3. No Preemption
4. Circular Wait

Our strategy breaks the **Circular Wait** condition.

### Why Circular Wait Cannot Happen

Suppose transactions lock accounts only in ascending order.

Example:

* Transaction T1 wants accounts `(10 → 20)`
* Transaction T2 wants accounts `(20 → 10)`

Both transactions acquire:

```text
Lock(10) before Lock(20)
```

A cycle would require:

```text
T1 waits for T2
T2 waits for T1
```

But this cannot occur because:

* No transaction can hold a higher-numbered lock while waiting for a lower-numbered lock
* The ordering forms a strict global hierarchy
* Lock acquisition always moves upward

Thus, the wait-for graph becomes acyclic.

Therefore:

```text
Circular Wait is impossible
```

and deadlock cannot occur.

---

## Advantages of Prevention

### Benefits

* Deterministic behavior
* No transaction rollback required
* Lower synchronization overhead
* Easier debugging
* Better throughput under moderate contention

### Tradeoff

Transactions may still wait for locks, but they will never deadlock permanently.

---

# 2. Buffer Pool Integration

## Buffer Pool Purpose

The buffer pool simulates loading account data from disk into memory while demonstrating the bounded-buffer producer-consumer problem using semaphores.

The pool contains a fixed number of slots:

```c
#define BUFFER_POOL_SIZE 5
```

Synchronization uses:

* `sem_t empty_slots`
* `sem_t full_slots`
* `pthread_mutex_t pool_lock`

---

## Buffer Pool Strategy

### Chosen Design

We use:

```text
Load on first access, unload on transaction commit
```

---

## When Accounts Are Loaded

An account is loaded into the buffer pool:

* The first time a transaction accesses the account
* Before any read or write operation occurs

Example:

```c
load_account(&buffer_pool, account_id);
```

This occurs before:

* DEPOSIT
* WITHDRAW
* TRANSFER
* BALANCE

---

## When Accounts Are Unloaded

Accounts are unloaded:

* After the transaction completes
* During COMMIT or ABORT cleanup

Example:

```c
unload_account(&buffer_pool, account_id);
```

---

## What Happens When the Pool Is Full

When no empty slot exists:

```c
sem_wait(&pool->empty_slots);
```

The transaction thread blocks until another transaction unloads an account.

This behavior demonstrates:

* Producer-consumer synchronization
* Resource contention
* Bounded resource management

---

## Correctness Justification

This strategy preserves correctness because:

* Transactions only access loaded accounts
* Buffer pool metadata is protected by `pool_lock`
* Semaphores prevent over-allocation
* Accounts remain loaded while actively used

The implementation avoids:

* Double allocation
* Race conditions
* Invalid memory access

---

## Performance Justification

### Why This Strategy Works Well

Compared to loading/unloading per operation:

* Reduces repeated buffer operations
* Reduces semaphore contention
* Improves cache locality

Compared to loading all accounts at transaction start:

* Avoids loading unused accounts
* Conserves limited buffer slots

Thus, it balances:

```text
Performance + Correctness + Resource Efficiency
```

---

# 3. Reader-Writer Lock Performance

## Synchronization Comparison

We benchmarked:

### Option 1

```c
pthread_mutex_t
```

### Option 2

```c
pthread_rwlock_t
```

---

## Benchmark Workload

The largest improvement appeared in:

```text
trace_readers.txt
```

This workload contains multiple concurrent BALANCE operations on the same account.

Example:

```text
T1 BALANCE 10
T2 BALANCE 10
T3 BALANCE 10
T4 BALANCE 10
```

---

## Benchmark Results

| Lock Type        | Completion Time | Concurrent Reads |
| ---------------- | --------------- | ---------------- |
| pthread_mutex_t  | 4 ticks         | No               |
| pthread_rwlock_t | 1 tick          | Yes              |

---

## Why `pthread_rwlock_t` Performs Better

### Mutex Behavior

A regular mutex allows:

```text
ONLY ONE THREAD
```

at a time, even for read-only operations.

Thus:

* Readers serialize unnecessarily
* Parallelism is lost

---

### Reader-Writer Lock Behavior

`pthread_rwlock_t` allows:

```text
MULTIPLE CONCURRENT READERS
```

while still enforcing exclusive writers.

Therefore:

* Multiple BALANCE operations proceed simultaneously
* Read-heavy workloads achieve higher throughput
* Lock contention decreases significantly

---

## Why Improvement Is Greatest on Read-Heavy Workloads

Banking systems often contain many:

* Balance inquiries
* Statement checks
* Read-only analytics

These operations do not modify data.

Reader-writer locks exploit this by allowing parallel reads while still preserving consistency.

For write-heavy workloads:

* Performance gains are smaller
* Writers still require exclusive access

---

## Correctness Benefits

Reader-writer locks preserve:

* Isolation
* Consistency
* Atomicity of writes

while improving concurrency for safe reads.

---

# 4. Timer Thread Design

## Why a Separate Timer Thread Is Necessary

The timer thread simulates a global clock shared by all transaction threads.

Example:

```c
global_tick++;
pthread_cond_broadcast(&tick_changed);
```

This allows transactions to:

* Start at different simulated times
* Execute concurrently
* Wait independently

Without a timer thread, true concurrent scheduling cannot be tested properly.

---

## What Would Break Without the Timer Thread

If operations were processed sequentially:

* Transactions would execute one-by-one
* No realistic lock contention would occur
* Deadlocks could not be reproduced
* Reader concurrency would never appear
* Wait times would always be near zero

The system would become:

```text
Single-threaded simulation
```

instead of a real concurrent banking system.

---

## How the Timer Thread Enables True Concurrency Testing

The timer thread enables:

### Simultaneous Transaction Start Times

Example:

```text
T3 starts at tick 2
T4 starts at tick 2
```

Both threads become runnable concurrently.

---

### Real Lock Contention

Multiple transactions compete for:

* Account locks
* Buffer pool slots
* Shared resources

This exposes real synchronization behavior.

---

### Accurate Waiting Metrics

Transactions measure:

```text
wait_ticks
```

while blocked on locks or resources.

Without concurrent execution, these metrics would be meaningless.

---

### Deadlock Scenarios

Concurrent transfers can overlap:

```text
T1: transfer 10 → 20
T2: transfer 20 → 10
```

This allows testing of:

* Lock ordering
* Deadlock prevention correctness

---

## Additional Benefits

The timer thread also provides:

* Deterministic scheduling
* Reproducible benchmarks
* Controlled concurrency experiments
* Easier debugging

---

# Conclusion

This concurrent banking system demonstrates real-world concurrency control techniques used in database systems and financial applications.

The design combines:

* POSIX threads
* Reader-writer synchronization
* Deadlock prevention
* Semaphores
* Bounded buffers
* Concurrent scheduling

to provide a correct, thread-safe, and performant banking simulation.

The implementation ensures:

* Zero data races under ThreadSanitizer
* Deadlock-free execution
* Serializable account updates
* Efficient read concurrency
* Proper resource synchronization

while maintaining modularity and clean systems programming practices.
