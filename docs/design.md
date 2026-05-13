# Concurrent Banking System Design Documentation

---

# 1. Deadlock Strategy Choice

## Chosen Strategy

The system uses:

```text
Deadlock Prevention via Lock Ordering
```

instead of deadlock detection.

---

## Why We Chose Prevention

We selected deadlock prevention because it is simpler, deterministic, and introduces less runtime overhead than deadlock detection.

The banking system frequently performs transfer operations that require locking two accounts simultaneously. Without proper synchronization, two transactions transferring money in opposite directions could deadlock permanently.

Example:

```text
T1: transfer(10 → 20)
T2: transfer(20 → 10)
```

If T1 locks account 10 while T2 locks account 20, both transactions could wait forever for the other lock.

Lock ordering eliminates this possibility entirely before deadlock can occur.

Additional reasons for choosing prevention:

- Easier to implement correctly
- Lower computational overhead
- No wait-for graph maintenance required
- No transaction rollback required
- Better predictability during testing

---

## Lock Ordering Strategy

All transfer operations acquire locks in ascending account ID order.

Implementation:

```c
int first = (acc1 < acc2) ? acc1 : acc2;
int second = (acc1 < acc2) ? acc2 : acc1;
```

Then:

```c
pthread_rwlock_wrlock(&bank.accounts[first].lock);
pthread_rwlock_wrlock(&bank.accounts[second].lock);
```

This ordering is applied consistently across every transaction.

---

## Proof That Lock Ordering Eliminates Circular Wait

Deadlock requires all four Coffman conditions:

1. Mutual Exclusion
2. Hold and Wait
3. No Preemption
4. Circular Wait

Our implementation breaks the:

```text
Circular Wait
```

condition.

Because locks are always acquired in ascending order:

```text
10 before 20
5 before 12
3 before 99
```

a transaction can never hold a higher-numbered lock while waiting for a lower-numbered lock.

Therefore:

- Wait relationships always move upward
- Cycles cannot form
- The wait-for graph becomes acyclic

Since circular wait is impossible, deadlock cannot occur.

---

# 2. Buffer Pool Integration

## Buffer Pool Design

The banking system includes a bounded buffer pool implemented using semaphores.

Pool size:

```c
#define BUFFER_POOL_SIZE 5
```

Synchronization primitives used:

- `sem_t empty_slots`
- `sem_t full_slots`
- `pthread_mutex_t pool_lock`

---

## When Accounts Are Loaded

Accounts are loaded into the buffer pool:

```text
On first access during a transaction
```

Implementation:

```c
load_account_once(loaded_accounts, account_id);
```

This occurs before:

- DEPOSIT
- WITHDRAW
- TRANSFER
- BALANCE

For transfer operations, both source and destination accounts are loaded.

---

## When Accounts Are Unloaded

Accounts remain in the buffer pool during the entire transaction.

They are unloaded:

```text
When the transaction commits or aborts
```

Implementation:

```c
unload_loaded_accounts(loaded_accounts);
```

This strategy minimizes repeated buffer operations and reduces semaphore contention.

---

## What Happens When the Pool Is Full

If the buffer pool has no available slots:

```c
sem_wait(&pool->empty_slots);
```

the requesting transaction blocks until another transaction unloads an account.

The implementation also tracks blocked operations:

```c
buffer_blocked_ops++;
```

This demonstrates the classical producer-consumer synchronization problem.

---

## Performance Justification

The chosen strategy provides a balance between performance and correctness.

### Advantages

- Avoids repeated loading/unloading
- Reduces semaphore overhead
- Improves cache locality
- Keeps active transaction data available

### Compared to Per-Operation Loading

Loading/unloading per operation would:

- Increase semaphore contention
- Increase synchronization overhead
- Reduce throughput

### Compared to Loading Everything at Start

Loading all accounts at transaction start could waste buffer slots on unused accounts.

The chosen design only loads accounts when needed.

---

## Correctness Justification

Correctness is maintained because:

- Buffer metadata is protected by `pool_lock`
- Semaphores prevent over-allocation
- Accounts remain valid while used
- Transactions cannot exceed pool capacity

This prevents:

- Race conditions
- Double allocation
- Invalid access
- Resource corruption

---

# 3. Reader-Writer Lock Performance

## Synchronization Comparison

The system was benchmarked using:

### Mutex Version

```c
pthread_mutex_t
```

### Reader-Writer Version

```c
pthread_rwlock_t
```

---

## Benchmark Workload

The largest improvement occurred using:

```text
trace_readers.txt
```

This workload performs multiple concurrent balance inquiries on the same account.

Example:

```text
T1 BALANCE 10
T2 BALANCE 10
T3 BALANCE 10
T4 BALANCE 10
```

---

## Benchmark Results

| Lock Type | Completion Time | Concurrent Reads |
|---|---|---|
| pthread_mutex_t | 4 ticks | No |
| pthread_rwlock_t | 1 tick | Yes |

---

## Why Reader-Writer Locks Improve Performance

A normal mutex allows only one thread at a time, even for read-only operations.

Therefore:

- Readers serialize unnecessarily
- Threads wait even when no data modification occurs

Reader-writer locks solve this by allowing:

```text
Multiple simultaneous readers
```

while still preserving exclusive writers.

This significantly improves throughput on read-heavy workloads.

---

## Why Improvement Is Largest on Read-Heavy Workloads

Balance inquiries do not modify account data.

Using `pthread_rwlock_t` allows multiple BALANCE operations to execute simultaneously.

This reduces:

- Lock contention
- Waiting time
- Idle CPU time

Write-heavy workloads see smaller improvements because writers still require exclusive access.

---

## Correctness Benefits

Reader-writer locks still preserve:

- Data consistency
- Isolation
- Atomic updates

while improving safe concurrency.

---

# 4. Timer Thread Design

## Purpose of the Timer Thread

The system uses a dedicated timer thread to simulate a global clock.

Implementation:

```c
global_tick++;
pthread_cond_broadcast(&tick_changed);
```

Transactions wait for their scheduled start times using:

```c
wait_until_tick(target_tick);
```

---

## Why a Separate Timer Thread Is Necessary

Without a timer thread:

- Transactions would execute sequentially
- No real concurrency would occur
- Lock contention would disappear
- Deadlock scenarios could not be tested

The timer thread creates realistic overlapping execution windows.

---

## What Would Break Without the Timer Thread

If operations were processed sequentially:

- Reader-writer concurrency would never appear
- Buffer pool contention would disappear
- Deadlock prevention logic would never be exercised
- Wait tick metrics would become meaningless

The system would become a single-threaded simulation instead of a concurrent system.

---

## How the Timer Thread Enables True Concurrency Testing

The timer thread enables multiple transactions to become runnable simultaneously.

Example:

```text
T1 starts at tick 0
T2 starts at tick 0
T3 starts at tick 0
```

This creates:

- Real lock contention
- Concurrent reads
- Competing transfers
- Semaphore blocking

The timer thread also enables:

- Deterministic scheduling
- Repeatable benchmarks
- Accurate waiting metrics
- Controlled concurrency experiments

---

# Conclusion

This project demonstrates practical concurrency control techniques used in real database and banking systems.

The implementation combines:

- POSIX threads
- Reader-writer synchronization
- Semaphores
- Deadlock prevention
- Concurrent scheduling
- Shared resource management

to create a correct and thread-safe banking simulation.

The design prioritizes:

- Correctness
- Predictable synchronization
- Deadlock-free execution
- Read concurrency performance
- Clean modular architecture