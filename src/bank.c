#include <stdio.h>

#include "bank.h"
#include "lock_mgr.h"

Bank bank;

/*
 * Initialize bank structures.
 */
void init_bank(void) {

    bank.num_accounts = 0;

    pthread_mutex_init(&bank.bank_lock, NULL);
}

/*
 * Load accounts from file.
 */
bool load_accounts(const char* filename) {

    /* TODO */

    return true;
}

/*
 * Read balance safely.
 */
int get_balance(int account_id) {

    Account* acc = &bank.accounts[account_id];

    pthread_rwlock_rdlock(&acc->lock);

    int balance = acc->balance_centavos;

    pthread_rwlock_unlock(&acc->lock);

    return balance;
}

/*
 * Deposit money safely.
 */
void deposit(int account_id, int amount_centavos) {

    Account* acc = &bank.accounts[account_id];

    pthread_rwlock_wrlock(&acc->lock);

    acc->balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&acc->lock);
}

/*
 * Withdraw money safely.
 */
bool withdraw(int account_id, int amount_centavos) {

    Account* acc = &bank.accounts[account_id];

    pthread_rwlock_wrlock(&acc->lock);

    if (acc->balance_centavos < amount_centavos) {
        pthread_rwlock_unlock(&acc->lock);
        return false;
    }

    acc->balance_centavos -= amount_centavos;

    pthread_rwlock_unlock(&acc->lock);

    return true;
}

/*
 * Transfer money using lock ordering.
 */
bool transfer(int from_id, int to_id, int amount_centavos) {

    lock_accounts_ordered(from_id, to_id);

    Account* from_acc = &bank.accounts[from_id];
    Account* to_acc = &bank.accounts[to_id];

    if (from_acc->balance_centavos < amount_centavos) {
        unlock_accounts_ordered(from_id, to_id);
        return false;
    }

    from_acc->balance_centavos -= amount_centavos;
    to_acc->balance_centavos += amount_centavos;

    unlock_accounts_ordered(from_id, to_id);

    return true;
}

void print_all_accounts(void) {

    /* TODO */
}

int total_bank_money(void) {

    /* TODO */

    return 0;
}