#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "transaction.h"
#include "timer.h"
#include "buffer_pool.h"
#include "metrics.h"

int main(int argc, char* argv[]) {

    printf("=== Concurrent Banking System ===\n");

    init_bank();

    init_buffer_pool(&buffer_pool);

    /* TODO:
     * Parse CLI arguments
     * Load accounts
     * Load transactions
     * Create timer thread
     * Create transaction threads
     * Join threads
     * Print metrics
     */

    return 0;
}