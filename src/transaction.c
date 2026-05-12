#include <stdio.h>
#include <string.h>

#include "transaction.h"
#include "timer.h"
#include "bank.h"
#include "buffer_pool.h"

Transaction transactions[MAX_TRANSACTIONS];

int num_transactions = 0;

__thread int current_tx_id = -1;

static Transaction* find_transaction(int tx_id) {

    for (int i = 0; i < num_transactions; i++) {

        if (transactions[i].tx_id == tx_id) {
            return &transactions[i];
        }
    }

    return NULL;
}

static Transaction* create_transaction(int tx_id, int start_tick) {

    if (num_transactions >= MAX_TRANSACTIONS) {
        return NULL;
    }

    Transaction* tx = &transactions[num_transactions++];

    tx->tx_id = tx_id;
    tx->num_ops = 0;
    tx->start_tick = start_tick;
    tx->actual_start = -1;
    tx->actual_end = -1;
    tx->wait_ticks = 0;
    tx->status = TX_RUNNING;

    return tx;
}

static void load_account_once(bool loaded_accounts[], int account_id) {

    if (account_id < 0 || account_id >= MAX_ACCOUNTS) {
        return;
    }

    if (!loaded_accounts[account_id]) {
        load_account(&buffer_pool, account_id);
        loaded_accounts[account_id] = true;
    }
}

static void unload_loaded_accounts(bool loaded_accounts[]) {

    for (int account_id = 0; account_id < MAX_ACCOUNTS; account_id++) {

        if (loaded_accounts[account_id]) {
            unload_account(&buffer_pool, account_id);
            loaded_accounts[account_id] = false;
        }
    }
}

/*
 * Parse trace file.
 */
bool load_transactions(const char* filename) {

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        return false;
    }

    num_transactions = 0;

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {

        /* Skip blank lines and comment lines starting with '#' (possibly preceded by whitespace) */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\0') {
            continue;
        }

        int tx_id;
        int start_tick;
        char op_name[16];
        int account_id;
        int amount_centavos;
        int target_account = -1;

        int parsed = sscanf(line,
                            "T%d %d %15s %d %d",
                            &tx_id,
                            &start_tick,
                            op_name,
                            &account_id,
                            &amount_centavos);

        if (parsed < 4) {
            fclose(file);
            return false;
        }

        Transaction* tx = find_transaction(tx_id);

        if (tx == NULL) {
            tx = create_transaction(tx_id, start_tick);
            if (tx == NULL) {
                fclose(file);
                return false;
            }
        }

        if (start_tick < tx->start_tick) {
            tx->start_tick = start_tick;
        }

        if (tx->num_ops >= MAX_OPERATIONS) {
            fclose(file);
            return false;
        }

        Operation* op = &tx->ops[tx->num_ops++];

        if (strcmp(op_name, "DEPOSIT") == 0) {
            if (parsed < 5) {
                fclose(file);
                return false;
            }

            op->type = OP_DEPOSIT;
            op->account_id = account_id;
            op->amount_centavos = amount_centavos;
            op->target_account = -1;
        } else if (strcmp(op_name, "WITHDRAW") == 0) {
            if (parsed < 5) {
                fclose(file);
                return false;
            }

            op->type = OP_WITHDRAW;
            op->account_id = account_id;
            op->amount_centavos = amount_centavos;
            op->target_account = -1;
        } else if (strcmp(op_name, "TRANSFER") == 0) {
            int transfer_parsed = sscanf(line,
                                         "T%d %d %15s %d %d %d",
                                         &tx_id,
                                         &start_tick,
                                         op_name,
                                         &account_id,
                                         &target_account,
                                         &amount_centavos);

            if (transfer_parsed != 6) {
                fclose(file);
                return false;
            }

            op->type = OP_TRANSFER;
            op->account_id = account_id;
            op->amount_centavos = amount_centavos;
            op->target_account = target_account;
        } else if (strcmp(op_name, "BALANCE") == 0) {
            op->type = OP_BALANCE;
            op->account_id = account_id;
            op->amount_centavos = 0;
            op->target_account = -1;
        } else {
            fclose(file);
            return false;
        }
    }

    fclose(file);

    return true;
}

/*
 * Transaction execution thread.
 */
void* execute_transaction(void* arg) {

    Transaction* tx = (Transaction*)arg;
    bool loaded_accounts[MAX_ACCOUNTS] = { false };

    /* Set thread-local tx id for logging in lock manager */
    current_tx_id = tx->tx_id;

    wait_until_tick(tx->start_tick);

    tx->actual_start = global_tick;

    tx->status = TX_RUNNING;
    tx->wait_ticks = tx->actual_start - tx->start_tick;

    /* Print transaction started with first operation summary */
    if (tx->num_ops > 0) {
        Operation* first_op = &tx->ops[0];

        switch (first_op->type) {
            case OP_DEPOSIT:
                print_log("  T%d started: DEPOSIT account %d amount PHP %d.%02d\n",
                          tx->tx_id,
                          first_op->account_id,
                          first_op->amount_centavos / 100,
                          first_op->amount_centavos % 100);
                break;
            case OP_WITHDRAW:
                print_log("  T%d started: WITHDRAW account %d amount PHP %d.%02d\n",
                          tx->tx_id,
                          first_op->account_id,
                          first_op->amount_centavos / 100,
                          first_op->amount_centavos % 100);
                break;
            case OP_TRANSFER:
                print_log("  T%d started: TRANSFER from %d to %d amount PHP %d.%02d\n",
                          tx->tx_id,
                          first_op->account_id,
                          first_op->target_account,
                          first_op->amount_centavos / 100,
                          first_op->amount_centavos % 100);
                break;
            case OP_BALANCE:
                print_log("  T%d started: BALANCE account %d\n",
                          tx->tx_id,
                          first_op->account_id);
                break;
        }
    }

    for (int i = 0; i < tx->num_ops; i++) {

        Operation* op = &tx->ops[i];
        int tick_before = global_tick;

        load_account_once(loaded_accounts, op->account_id);

        if (op->type == OP_TRANSFER) {
            load_account_once(loaded_accounts, op->target_account);
        }

        switch (op->type) {

            case OP_DEPOSIT:
                deposit(op->account_id,
                        op->amount_centavos);
                print_log("  T%d completed: DEPOSIT successful\n", tx->tx_id);
                break;

            case OP_WITHDRAW:

                if (!withdraw(op->account_id,
                              op->amount_centavos)) {

                    tx->status = TX_ABORTED;
                    print_log("  T%d completed: WITHDRAW aborted\n", tx->tx_id);
                    return NULL;
                }

                print_log("  T%d completed: WITHDRAW successful\n", tx->tx_id);

                break;

            case OP_TRANSFER:

                if (!transfer(op->account_id,
                              op->target_account,
                              op->amount_centavos)) {

                    tx->status = TX_ABORTED;
                    print_log("  T%d completed: TRANSFER aborted\n", tx->tx_id);
                    return NULL;
                }

                print_log("  T%d completed: TRANSFER successful\n", tx->tx_id);

                break;

            case OP_BALANCE: {

                int balance = get_balance(op->account_id);

                printf("T%d: Account %d balance = PHP %d.%02d\n",
                       tx->tx_id,
                       op->account_id,
                       balance / 100,
                       balance % 100);

                  print_log("  T%d completed: BALANCE printed\n", tx->tx_id);

                break;
            }
        }

        tx->wait_ticks += (global_tick - tick_before);
    }

    unload_loaded_accounts(loaded_accounts);

    tx->actual_end = global_tick;

    tx->status = TX_COMMITTED;

    return NULL;
}