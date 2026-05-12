#include <stdio.h>

#include "bank.h"
#include "lock_mgr.h"

Bank bank;

static bool is_valid_account_id(int account_id) {

    return account_id >= 0 && account_id < MAX_ACCOUNTS &&
           bank.accounts[account_id].account_id == account_id;
}

/*
 * Initialize bank structures.
 */
void init_bank(void) {

    bank.num_accounts = 0;

    pthread_mutex_init(&bank.bank_lock, NULL);

    for (int i = 0; i < MAX_ACCOUNTS; i++) {

        bank.accounts[i].account_id = -1;
        bank.accounts[i].balance_centavos = 0;
        pthread_rwlock_init(&bank.accounts[i].lock, NULL);
    }
}

/*
 * Load accounts from file.
 */
bool load_accounts(const char* filename) {

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        return false;
    }

    char line[256];
    int account_id;
    int balance_centavos;

    while (fgets(line, sizeof(line), file) != NULL) {

        /* Skip blank lines and comment lines starting with '#' (possibly preceded by whitespace) */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\0') {
            continue;
        }

        if (sscanf(line, "%d %d", &account_id, &balance_centavos) != 2) {
            fclose(file);
            return false;
        }

        if (account_id < 0 || account_id >= MAX_ACCOUNTS) {
            fclose(file);
            return false;
        }

        bank.accounts[account_id].account_id = account_id;
        bank.accounts[account_id].balance_centavos = balance_centavos;
        bank.num_accounts++;
    }

    fclose(file);

    return true;
}

/*
 * Read balance safely.
 */
int get_balance(int account_id) {

    if (!is_valid_account_id(account_id)) {
        return 0;
    }

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

    if (!is_valid_account_id(account_id)) {
        return;
    }

    Account* acc = &bank.accounts[account_id];

    pthread_rwlock_wrlock(&acc->lock);

    acc->balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&acc->lock);
}

/*
 * Withdraw money safely.
 */
bool withdraw(int account_id, int amount_centavos) {

    if (!is_valid_account_id(account_id)) {
        return false;
    }

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

    if (from_id == to_id) {
        return is_valid_account_id(from_id);
    }

    if (!is_valid_account_id(from_id) || !is_valid_account_id(to_id)) {
        return false;
    }

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

    for (int i = 0; i < MAX_ACCOUNTS; i++) {

        if (!is_valid_account_id(i)) {
            continue;
        }

        printf("Account %d: %d centavos\n",
               i,
               get_balance(i));
    }
}

int total_bank_money(void) {

    int total = 0;

    for (int i = 0; i < MAX_ACCOUNTS; i++) {

        if (is_valid_account_id(i)) {
            total += get_balance(i);
        }
    }

    return total;
}