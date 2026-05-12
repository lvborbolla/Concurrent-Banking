#include <stdio.h>

#include "transaction.h"
#include "timer.h"
#include "bank.h"

Transaction transactions[MAX_TRANSACTIONS];

int num_transactions = 0;

/*
 * Parse trace file.
 */
bool load_transactions(const char* filename) {

    /* TODO */

    return true;
}

/*
 * Transaction execution thread.
 */
void* execute_transaction(void* arg) {

    Transaction* tx = (Transaction*)arg;

    wait_until_tick(tx->start_tick);

    tx->actual_start = global_tick;

    tx->status = TX_RUNNING;

    for (int i = 0; i < tx->num_ops; i++) {

        Operation* op = &tx->ops[i];

        switch (op->type) {

            case OP_DEPOSIT:
                deposit(op->account_id,
                        op->amount_centavos);
                break;

            case OP_WITHDRAW:

                if (!withdraw(op->account_id,
                              op->amount_centavos)) {

                    tx->status = TX_ABORTED;
                    return NULL;
                }

                break;

            case OP_TRANSFER:

                if (!transfer(op->account_id,
                              op->target_account,
                              op->amount_centavos)) {

                    tx->status = TX_ABORTED;
                    return NULL;
                }

                break;

            case OP_BALANCE: {

                int balance = get_balance(op->account_id);

                printf("T%d BALANCE: %d\n",
                       tx->tx_id,
                       balance);

                break;
            }
        }
    }

    tx->actual_end = global_tick;

    tx->status = TX_COMMITTED;

    return NULL;
}