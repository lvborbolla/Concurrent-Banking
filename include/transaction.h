#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_OPERATIONS 256
#define MAX_TRANSACTIONS 100

/*
 * Supported banking operations.
 */
typedef enum {
    OP_DEPOSIT,
    OP_WITHDRAW,
    OP_TRANSFER,
    OP_BALANCE
} OpType;

/*
 * Represents one operation inside a transaction.
 */
typedef struct {
    OpType type;
    int account_id;
    int amount_centavos;
    int target_account;
    int start_tick;
} Operation;

/*
 * Transaction execution status.
 */
typedef enum {
    TX_RUNNING,
    TX_COMMITTED,
    TX_ABORTED
} TxStatus;

/*
 * Transaction object.
 */
typedef struct {
    int tx_id;

    Operation ops[MAX_OPERATIONS];
    int num_ops;

    int start_tick;

    pthread_t thread;

    int actual_start;
    int actual_end;
    int wait_ticks;

    TxStatus status;

} Transaction;

/* Global transaction array */
extern Transaction transactions[MAX_TRANSACTIONS];
extern int num_transactions;

/* Current transaction id for the running thread (thread-local) */
extern __thread int current_tx_id;

/* Parse trace file */
bool load_transactions(const char* filename);

/* Thread routine */
void* execute_transaction(void* arg);

#endif