#include "lock_mgr.h"
#include "bank.h"

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

    pthread_rwlock_wrlock(&bank.accounts[second].lock);
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