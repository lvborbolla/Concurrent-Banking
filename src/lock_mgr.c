#include "lock_mgr.h"
#include "bank.h"
#include "transaction.h"
#include "utils.h"

/*
 * Lock accounts in ascending order.
 */
void lock_accounts_ordered(int acc1, int acc2) {

    if (acc1 == acc2) {
        pthread_rwlock_wrlock(&bank.accounts[acc1].lock);
        return;
    }

    int first = (acc1 < acc2) ? acc1 : acc2;
    int second = (acc1 < acc2) ? acc2 : acc1;

    pthread_rwlock_wrlock(&bank.accounts[first].lock);
    print_log("  T%d acquired lock on account %d\n", current_tx_id, first);

    /* Try to acquire second lock without blocking to detect ordering wait */
    if (pthread_rwlock_trywrlock(&bank.accounts[second].lock) != 0) {
        print_log("  [DEADLOCK PREVENTED] Lock ordering: T%d waiting for account %d\n", current_tx_id, second);
        pthread_rwlock_wrlock(&bank.accounts[second].lock);
    } else {
        print_log("  T%d acquired lock on account %d\n", current_tx_id, second);
    }
}

/*
 * Unlock accounts.
 */
void unlock_accounts_ordered(int acc1, int acc2) {

    if (acc1 == acc2) {
        pthread_rwlock_unlock(&bank.accounts[acc1].lock);
        return;
    }

    int first = (acc1 < acc2) ? acc1 : acc2;
    int second = (acc1 < acc2) ? acc2 : acc1;

    pthread_rwlock_unlock(&bank.accounts[second].lock);
    pthread_rwlock_unlock(&bank.accounts[first].lock);
}