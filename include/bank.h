#ifndef BANK_H
#define BANK_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 100

/*
 * Represents a single bank account.
 */
typedef struct {
    int account_id;
    int balance_centavos;
    pthread_rwlock_t lock;
} Account;

/*
 * Represents the banking database.
 */
typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;
    pthread_mutex_t bank_lock;
} Bank;

/* Global bank instance */
extern Bank bank;

/* Initialization */
void init_bank(void);

/* Load accounts from file */
bool load_accounts(const char* filename);

/* Account operations */
int get_balance(int account_id);

void deposit(int account_id, int amount_centavos);

bool withdraw(int account_id, int amount_centavos);

bool transfer(int from_id, int to_id, int amount_centavos);

/* Utility */
void print_all_accounts(void);

int total_bank_money(void);

#endif