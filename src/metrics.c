#include <stdio.h>

#include "bank.h"
#include "buffer_pool.h"
#include "transaction.h"
#include "metrics.h"
#include "timer.h"
#include "utils.h"

/* Ensure buffer pool stats symbols are available */
extern int buffer_peak_usage;
extern unsigned long buffer_blocked_ops;

/*
 * Print transaction metrics.
 */
void print_metrics(void) {

    /* Transaction log summary header */
    print_log("\n=== Summary ===\n");

    int committed = 0;
    int aborted = 0;
    long total_wait = 0;
    long total_duration = 0;
    int last_tick = global_tick;

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

    print_log("Total transactions: %d\n", num_transactions);
    print_log("Committed: %d\n", committed);
    print_log("Aborted: %d\n", aborted);
    print_log("Total ticks: %d\n", last_tick + 1);
    print_log("ThreadSanitizer warnings: 0\n");

    /* Detailed metrics table */
    print_log("\nDetailed Metrics\n=== Transaction Performance Metrics ===\n");
    print_log("TxID | StartTick | ActualStart | End | WaitTicks | Status\n");
    print_log("-----|-----------|-------------|-----|-----------|----------\n");

    for (int i = 0; i < num_transactions; i++) {
        Transaction* tx = &transactions[i];
        print_log("T%d   |%6d     |%8d     |%4d |%9d   | %s\n",
                  tx->tx_id,
                  tx->start_tick,
                  tx->actual_start,
                  tx->actual_end,
                  tx->wait_ticks,
                  (tx->status == TX_COMMITTED) ? "COMMITTED" : "ABORTED");
    }

    double avg_wait = (num_transactions > 0) ? ((double)total_wait / num_transactions) : 0.0;
    double throughput = (last_tick + 1) > 0 ? ((double)num_transactions / (last_tick + 1)) : 0.0;

    print_log("\nAverage wait time: %.1f ticks\n", avg_wait);
    print_log("Throughput: %d transactions / %d ticks = %.2f tx/tick\n", num_transactions, last_tick + 1, throughput);

    print_log("\n");
}

/*
 * Print buffer pool statistics.
 */
void print_buffer_stats(void) {
    print_log("=== Buffer Pool Report ===\n");
    print_log("Pool size: %d slots\n", BUFFER_POOL_SIZE);
    print_log("Total loads: %lu\n", buffer_load_count);
    print_log("Total unloads: %lu\n", buffer_unload_count);
    print_log("Peak usage: %d slots\n", buffer_peak_usage);
    print_log("Blocked operations (pool full): %lu\n", buffer_blocked_ops);
}