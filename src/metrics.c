#include <stdio.h>

#include "bank.h"
#include "buffer_pool.h"
#include "transaction.h"
#include "metrics.h"

/*
 * Print transaction metrics.
 */
void print_metrics(void) {

    printf("=== Metrics ===\n");
    int committed = 0;
    int aborted = 0;
    long total_wait = 0;
    long total_duration = 0;

    for (int i = 0; i < num_transactions; i++) {

        if (transactions[i].status == TX_COMMITTED) {
            committed++;
        } else if (transactions[i].status == TX_ABORTED) {
            aborted++;
        }

        if (transactions[i].actual_start >= 0 &&
            transactions[i].actual_end >= 0) {
            total_duration += transactions[i].actual_end - transactions[i].actual_start;
        }

        total_wait += transactions[i].wait_ticks;
    }

    printf("Transactions: %d\n", num_transactions);
    printf("Committed: %d\n", committed);
    printf("Aborted: %d\n", aborted);
    printf("Total wait ticks: %ld\n", total_wait);
    printf("Total active ticks: %ld\n", total_duration);
    printf("Total bank money: %d\n", total_bank_money());
}

/*
 * Print buffer pool statistics.
 */
void print_buffer_stats(void) {

    printf("=== Buffer Pool ===\n");
    printf("Current occupancy: %d/%d\n", buffer_current_occupancy, BUFFER_POOL_SIZE);
    printf("Loads: %lu\n", buffer_load_count);
    printf("Unloads: %lu\n", buffer_unload_count);
}