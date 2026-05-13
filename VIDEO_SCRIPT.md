# Video Script: Concurrent Banking System
## CMSC 125 Lab 3 - Demonstrating Concurrency, Locking, and Transaction Management

---

## **SEGMENT 1: Introduction (1-2 minutes)**

### Opening
"Hello! Today I'm going to walk you through a sophisticated concurrent banking system built in C. This project demonstrates critical concepts in concurrent programming: thread synchronization, deadlock prevention, and database transaction management. By the end of this video, you'll understand how multiple transactions can safely execute simultaneously on a shared bank database."

### Project Goals
"The system is designed to handle three main challenges:
1. **Concurrency** - Multiple transactions accessing accounts simultaneously
2. **Data Integrity** - Ensuring account balances stay consistent
3. **Deadlock Prevention** - Avoiding situations where threads wait for each other indefinitely

Let me show you what we're building."

---

## **SEGMENT 2: System Architecture Overview (2-3 minutes)**

### High-Level Architecture
"The system consists of five main components working together:

1. **Bank** - The core database with 100 accounts
2. **Transactions** - Operations that need to execute atomically
3. **Lock Manager** - Ensures safe concurrent access
4. **Buffer Pool** - Manages database page caching
5. **Timer** - Simulates real-time tick-based execution

Let me break down how they interact."

### The Bank Structure
"At the heart is the Bank structure. It maintains:
- A fixed array of 100 accounts
- Each account has:
  - An ID (0-99)
  - A balance stored in centavos for precision
  - A read-write lock for synchronization

The bank itself has a global mutex to protect structural changes."

### Transaction Structure
"Transactions are the workload. Each transaction contains:
- An ID for tracking
- Up to 256 operations (deposit, withdraw, transfer, or balance check)
- A start tick from the trace file
- Timing information for metrics
- A status: running, committed, or aborted

Transactions are loaded from trace files that simulate realistic banking operations."

### Key Data Structures
[*Show slides or screen with the header files*]
- **Account**: ID, balance, and read-write lock
- **Bank**: Array of accounts + mutex
- **Operation**: Type, accounts involved, amount, and timestamp
- **Transaction**: ID, operations, timing, status

---

## **SEGMENT 3: Core Components Deep Dive (4-5 minutes)**

### Component 1: Lock Manager - Deadlock Prevention
"The lock manager is crucial. Here's the problem it solves:

When we do a transfer from Account A to Account B, we need to lock both accounts. But imagine:
- Thread 1 locks Account A, then waits for Account B
- Thread 2 locks Account B, then waits for Account A
- Deadlock!

Our solution: **Ordered Locking**

We always acquire locks in the same order - sorted by account ID. This creates a strict ordering that prevents circular wait conditions."

*[Show code concept]*
```
lock_accounts_ordered(from_id, to_id):
  - Sort: smaller ID first
  - Acquire lock on lower ID account
  - Acquire lock on higher ID account
```

"This simple rule eliminates deadlock. All threads follow the same ordering, so we can never have a cycle."

### Component 2: The Timer Thread
"One thread acts as a global timer. Every fixed interval (configurable), it increments a tick counter. This simulates real-time progression and is used by operations to know when to execute.

Think of it like a heartbeat of the system - all transactions synchronize to these ticks."

### Component 3: Buffer Pool
"The buffer pool manages page caching for potential disk I/O. While this implementation uses in-memory accounts, the buffer pool demonstrates how a real database would cache frequently accessed pages.

It tracks:
- Page hits and misses
- Buffer space efficiency
- Access patterns for performance analysis"

### Component 4: Metrics Collection
"As transactions execute, the system collects metrics:
- Wait times (ticks spent waiting for locks)
- Actual execution times
- Success/abort counts
- Total money in system (data integrity check!)

These metrics tell us if our locking strategy is efficient."

---

## **SEGMENT 4: Execution Flow - The Main Loop (3-4 minutes)**

### Starting Up
"When the program starts, here's what happens:

**Step 1: Initialization**
- Initialize the bank with 100 empty accounts
- Set up the buffer pool
- Parse command-line arguments for configuration"

### Loading Data
"**Step 2: Load Initial State**
- Read accounts from a file (initial balances)
- Read transactions from a trace file (the workload)

These files define exactly what the test case will do."

### The Execution Model
"**Step 3: Launch Threads**
```
Create 1 Timer Thread
  ↓
Create N Transaction Threads (one per transaction)
```

Each transaction thread waits for its start tick, then executes operations sequentially."

### Transaction Execution (Critical Section)
"When a transaction runs its operations:

**For each operation:**
1. **DEPOSIT**: Acquire account write lock → add amount → release lock
2. **WITHDRAW**: Acquire account write lock → subtract amount (if sufficient) → release lock
3. **TRANSFER**: 
   - Acquire BOTH account locks (in sorted order!)
   - Deduct from source, add to target
   - Release both locks
4. **BALANCE**: Acquire account read lock → read balance → release lock

The key insight: locks are held for minimal time. We only lock what we need, for as long as we need."

### Shutdown and Reporting
"**Step 4: Join All Threads**
- Wait for all transactions to complete
- Stop the timer
- Print final metrics and account balances

The final total money in the bank proves data integrity - it should equal initial money!"

---

## **SEGMENT 5: Concurrency Features & Edge Cases (3-4 minutes)**

### Reader-Writer Locks
"Accounts use `pthread_rwlock_t` - a reader-writer lock.

Why this matters:
- **Multiple Readers** can hold the lock simultaneously (for BALANCE operations)
- **Exclusive Writer** blocks all readers (for DEPOSIT/WITHDRAW/TRANSFER)

This improves performance - many transactions can read account balances in parallel, only writers block each other."

### Trace-Based Simulation
"The system uses trace files to control execution timing:
```
TX_ID TICK OPERATION ACCOUNT [TARGET_AMOUNT]
1     0    DEPOSIT   5       100
2     5    TRANSFER  3       7 50
1     15   BALANCE   5
```

Each transaction starts when its tick arrives. This lets us:
- Reproduce exact execution scenarios
- Test specific concurrency patterns
- Verify specific edge cases like deadlock or race conditions"

### Atomicity Guarantee
"Each operation is atomic - operations don't interleave at the account level.

**But** - transactions are NOT fully serializable. A transaction might see the result of another transaction's operation in its middle. This is realistic for many database systems (dirty read possibility).

However, we maintain Account-level consistency - no account balance ever goes negative (for withdrawals), and money is never lost."

---

## **SEGMENT 6: Code Walkthrough - Key Functions (4-5 minutes)**

### The Transfer Operation (Most Complex)
"Let me show you the transfer function - it's where most concurrency concepts appear."

*[Display code on screen]*

```c
bool transfer(int from_id, int to_id, int amount_centavos) {
    // Acquire locks in sorted order - prevents deadlock!
    lock_accounts_ordered(from_id, to_id);
    
    Account* from = &bank.accounts[from_id];
    Account* to = &bank.accounts[to_id];
    
    // Check funds
    if (from->balance_centavos < amount_centavos) {
        unlock_accounts_ordered(from_id, to_id);
        return false;  // Insufficient funds
    }
    
    // Transfer atomically
    from->balance_centavos -= amount_centavos;
    to->balance_centavos += amount_centavos;
    
    unlock_accounts_ordered(from_id, to_id);
    return true;
}
```

"Notice:
1. **Lock Ordering**: Guaranteed deadlock prevention
2. **Atomicity**: Between lock and unlock, operations are indivisible
3. **Validation**: Check preconditions while locked
4. **Minimal Critical Section**: Release locks immediately after work is done"

### Transaction Executor Thread
"Each transaction runs in its own thread via `execute_transaction()`"

*[Display concept]*

```c
void* execute_transaction(void* arg) {
    Transaction* tx = (Transaction*)arg;
    
    // Wait until start tick
    while (current_tick < tx->start_tick) {
        pthread_yield();  // Give other threads time
    }
    
    tx->actual_start = current_tick;
    
    // Execute each operation
    for (int i = 0; i < tx->num_ops; i++) {
        Operation* op = &tx->ops[i];
        
        switch(op->type) {
            case OP_DEPOSIT:
                deposit(op->account_id, op->amount_centavos);
                break;
            case OP_TRANSFER:
                transfer(op->account_id, op->target_account, op->amount_centavos);
                break;
            // ... other operations
        }
    }
    
    tx->status = TX_COMMITTED;
    return NULL;
}
```

"Key points:
- Busy-wait until the start tick (in a real system, we'd use condition variables)
- Execute operations in sequence
- No buffering between operations within a transaction
- Marks as committed upon successful completion"

---

## **SEGMENT 7: Running a Test Case (2-3 minutes)**

### Building and Running
"Let's build and run the system. The Makefile has everything set up:"

*[Show terminal]*

```bash
$ make clean && make
gcc -Wall -Werror -pthread -o bankdb src/*.c ...

$ ./bankdb -a tests/accounts.txt -t tests/trace_simple.txt -i 10
```

### Arguments Explained
- `-a`: Initial accounts file
- `-t`: Transaction trace file
- `-i`: Tick interval in milliseconds

### Sample Output
```
=== Banking System Execution Log ===

Tick 0:
  [TX 1] Starting...

Tick 1:
  [TX 1] DEPOSIT to account 5: +100 centavos
  Account 5 balance: 100

Tick 2:
  [TX 2] TRANSFER: Account 3 → Account 7: 50 centavos
  Account 3 balance: 200 - 50 = 150
  Account 7 balance: 50 + 50 = 100
  
...

=== Final Metrics ===
Total transactions: 2
Committed: 2
Aborted: 0
Total wait time: 15 ticks
Average wait time: 7.5 ticks
Total bank money: 1000 centavos (consistent!)
```

### Interpreting Results
"The output shows:
- **Exact operation timing** - When each op executes relative to ticks
- **Account state changes** - Transparency into what's happening
- **Metrics** - Performance and correctness indicators
- **Final bank state** - Verification that data integrity is maintained"

---

## **SEGMENT 8: Advanced Test Cases (3-4 minutes)**

### Test Case 1: Simple Concurrent Deposits
*File: trace_simple.txt*

"Multiple transactions deposit to different accounts. No contention, so high concurrency.
Expected: All operations execute quickly with minimal waiting."

### Test Case 2: Deadlock Scenario
*File: trace_deadlock.txt*

"Specifically designed to cause deadlock WITHOUT ordered locking:
- TX 1: Transfer from Account 1 → 2
- TX 2: Transfer from Account 2 → 1

Without ordered locking: Deadlock.
With ordered locking: Both complete successfully.

This demonstrates why lock ordering is critical."

### Test Case 3: Reader-Writer Lock Contention
*File: trace_readers.txt*

"Many balance queries (reads) mixed with occasional transfers (writes).
Expected: Readers don't block each other, but writers still maintain consistency.
Metrics show high throughput due to read parallelism."

### Test Case 4: Abort Simulation
*File: trace_abort.txt*

"Transactions attempting to withdraw more than available.
Expected: Some transactions abort with insufficient funds.
Final metrics show both successful and failed transactions."

---

## **SEGMENT 9: Key Learnings & Design Patterns (2-3 minutes)**

### Pattern 1: Ordered Lock Acquisition
"**Problem**: Deadlock from circular lock dependencies
**Solution**: Always acquire locks in a consistent order
**Benefit**: Eliminates deadlock completely and is easy to understand
**Trade-off**: Requires knowing all locks needed upfront"

### Pattern 2: Reader-Writer Locks
"**Problem**: Many reads, few writes
**Solution**: Use `pthread_rwlock_t` instead of mutex
**Benefit**: Multiple readers can proceed simultaneously
**Trade-off**: Slightly higher overhead per lock operation"

### Pattern 3: Minimal Critical Sections
"**Problem**: Long-held locks reduce concurrency
**Solution**: Minimize operations between lock and unlock
**Benefit**: Better performance and responsiveness
**Trade-off**: Requires careful design to identify what must be locked"

### Pattern 4: Trace-Based Testing
"**Problem**: Hard to reproduce and test concurrency bugs
**Solution**: Use deterministic trace files with tick numbers
**Benefit**: Exact reproducibility and controlled scenarios
**Trade-off**: Tests don't discover unexpected interleavings (but don't need to!)"

---

## **SEGMENT 10: Conclusion & Extensions (1-2 minutes)**

### Summary
"We've built a production-inspired concurrent system that:
- ✓ Safely handles concurrent access to shared data
- ✓ Prevents deadlock with ordered locking
- ✓ Uses appropriate synchronization primitives
- ✓ Maintains data integrity with metrics
- ✓ Supports testing and debugging with traces"

### Real-World Applications
"These concepts are used in:
- **Database Systems** (PostgreSQL, MySQL) - MVCC, lock managers
- **Banking Systems** - Atomic transactions, money integrity
- **File Systems** - Buffer pools, page caching
- **Web Servers** - Thread pools, concurrent request handling"

### Possible Extensions
1. **Optimistic Locking** - Use version numbers instead of locks
2. **Transaction Logging** - Write-ahead logs for crash recovery
3. **Snapshot Isolation** - Allow transactions to see consistent snapshots
4. **Distributed Transactions** - Extend to multiple databases (two-phase commit)
5. **Performance Tuning** - Experiment with lock granularity, batch operations

### Final Thought
"The beauty of concurrent systems is that complexity hides behind simple abstractions. A `lock_accounts_ordered()` call hides the deep reasoning that prevents deadlock. Understanding these layers - from the simple ordered locking rule to the complete system behavior - is the mark of a good systems programmer.

Thank you for watching! Questions or comments are welcome."

---

## **SEGMENT 11: Live Demonstration Script (Optional, ~5-10 minutes)**

### Demo Part 1: Building
```bash
$ cd /path/to/Concurrent-Banking
$ make clean
$ make
$ ls -la bankdb
```
*Verify executable is created*

### Demo Part 2: Simple Test
```bash
$ ./bankdb -a tests/accounts.txt -t tests/trace_simple.txt -i 100
```
*Show output, highlight timing and operations*

### Demo Part 3: Metrics Analysis
"Look at the metrics output:
- What's the average wait time?
- Are any transactions aborted?
- Is total money conserved?"

### Demo Part 4: Trace Analysis
"Look at the trace file to understand what should happen:"
```bash
$ cat tests/trace_simple.txt
```
*Correlate trace file to actual execution*

### Demo Part 5: Code Review in Editor
"Let me show you key code sections:
- Start with transaction.h - understand the data structures
- Show lock_mgr.c - the deadlock prevention logic
- Show the transfer() function - where it all comes together
- Show main() - how everything orchestrates"

---

## **Visual Aid Suggestions**

### Diagrams to Create/Show:
1. **System Architecture Diagram**
   - Bank (center)
   - Transaction threads (surrounding)
   - Timer thread
   - Buffer pool (side)
   - Data flow arrows

2. **Lock Ordering Illustration**
   - Before: Thread 1→A→B, Thread 2→B→A (Deadlock!)
   - After: Both threads follow A→B order (No deadlock!)

3. **State Diagram for Transactions**
   - Start → Running → Committed/Aborted

4. **Memory Layout**
   - Bank struct
   - Account array
   - Per-account locks

5. **Timeline Example**
   - Show multiple transactions executing across time
   - Highlight when locks are held
   - Show tick progression

---

## **Timing Breakdown**

- **Introduction**: 1-2 minutes
- **Architecture Overview**: 2-3 minutes
- **Components Deep Dive**: 4-5 minutes
- **Execution Flow**: 3-4 minutes
- **Concurrency Features**: 3-4 minutes
- **Code Walkthrough**: 4-5 minutes
- **Running Tests**: 2-3 minutes
- **Advanced Test Cases**: 3-4 minutes
- **Key Learnings**: 2-3 minutes
- **Conclusion**: 1-2 minutes
- **Live Demo** (optional): 5-10 minutes

**Total: ~30-40 minutes (without demo) or ~40-50 minutes (with demo)**

---

## **Recording Tips**

1. **Use a screen recorder** (OBS Studio, ScreenFlow, etc.)
2. **Zoom to 125-150%** to make code readable
3. **Syntax highlighting** helps - use a nice theme
4. **Pause between segments** to let viewers catch up
5. **Read code slowly** - explain while reading
6. **Use pointer/highlighting** to emphasize code sections
7. **Show execution output** - let the system speak for itself
8. **Test all commands beforehand** - no surprises!
9. **Speak clearly** - technical content is dense, clarity helps
10. **Consider subtitles** - especially for code sections

---

**End of Script**
