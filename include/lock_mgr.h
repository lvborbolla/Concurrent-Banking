#ifndef LOCK_MGR_H
#define LOCK_MGR_H

/*
 * Deadlock prevention utilities.
 */

/* Acquire transfer locks using ordering */
void lock_accounts_ordered(int acc1, int acc2);

/* Release ordered locks */
void unlock_accounts_ordered(int acc1, int acc2);

#endif