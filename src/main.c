#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "bank.h"
#include "transaction.h"
#include "timer.h"
#include "buffer_pool.h"
#include "metrics.h"
#include "utils.h"

int main(int argc, char* argv[]) {

    printf("=== Banking System Execution Log ===\n");

    init_bank();

    init_buffer_pool(&buffer_pool);

    if (!parse_arguments(argc, argv)) {
        return 1;
    }

    if (accounts_file != NULL) {
        if (!load_accounts(accounts_file)) {
            fprintf(stderr, "Failed to load accounts from %s\n", accounts_file);
            return 1;
        }
    }

    if (!load_transactions(trace_file)) {
        fprintf(stderr, "Failed to load transactions from %s\n", trace_file);
        return 1;
    }

    int initial_total = total_bank_money();

    pthread_t timer;
    pthread_t tx_threads[MAX_TRANSACTIONS];

    /* Print initial Tick 0 before starting timer */
    if (verbose) {
        print_log("Timer thread started (tick interval: %dms)\n\n", tick_interval_ms);
        print_log("Tick 0:\n");
    }

    if (pthread_create(&timer, NULL, timer_thread, &tick_interval_ms) != 0) {
        fprintf(stderr, "Failed to start timer thread\n");
        return 1;
    }

    for (int i = 0; i < num_transactions; i++) {
        if (pthread_create(&tx_threads[i], NULL, execute_transaction, &transactions[i]) != 0) {
            fprintf(stderr, "Failed to start transaction thread %d\n", transactions[i].tx_id);
            simulation_running = false;
            pthread_join(timer, NULL);
            return 1;
        }
    }

    for (int i = 0; i < num_transactions; i++) {
        pthread_join(tx_threads[i], NULL);
    }

    simulation_running = false;
    pthread_join(timer, NULL);

    int final_total = total_bank_money();
    long net_delta = get_total_net_delta();
    bool conservation_passed = (final_total == initial_total + net_delta);

    print_log("Initial total: %d\n", initial_total);
    print_log("Final total: %d\n", final_total);
    print_log("Conservation check: %s\n", conservation_passed ? "PASSED" : "FAILED");

    print_metrics();
    print_buffer_stats();
    print_all_accounts();

    return 0;
}