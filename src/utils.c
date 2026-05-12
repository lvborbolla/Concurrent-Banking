#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include "utils.h"
#include <stdarg.h>

const char* accounts_file = NULL;
const char* trace_file = NULL;
const char* deadlock_strategy = "prevention";
int tick_interval_ms = 100;
bool verbose = false;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

static void print_usage(const char* program_name) {

    fprintf(stderr,
            "Usage: %s [OPTIONS]\n\n",
            program_name);
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "  --accounts=FILE              Initial account balances (optional)\n");
    fprintf(stderr, "  --trace=FILE                 Transaction workload (default: tests/trace_simple.txt)\n");
    fprintf(stderr, "  --deadlock=STR               Deadlock strategy: prevention|detection (default: prevention)\n");
    fprintf(stderr, "  --tick-ms=N                  Milliseconds per tick (default: 100)\n");
    fprintf(stderr, "  --verbose                    Print detailed operation logs\n");
    fprintf(stderr, "\nNote: Only 'prevention' deadlock strategy is implemented.\n");
}

/*
 * Parse command-line arguments.
 */
bool parse_arguments(int argc, char* argv[]) {

    accounts_file = NULL;
    trace_file = "tests/trace_simple.txt";
    deadlock_strategy = "prevention";
    tick_interval_ms = 100;
    verbose = false;

    for (int i = 1; i < argc; i++) {

        if (strncmp(argv[i], "--accounts=", 11) == 0) {
            accounts_file = argv[i] + 11;
        } else if (strncmp(argv[i], "--trace=", 8) == 0) {
            trace_file = argv[i] + 8;
        } else if (strncmp(argv[i], "--tick-ms=", 10) == 0 || strncmp(argv[i], "--tick=", 7) == 0) {
            /* Accept both --tick-ms= and legacy --tick= */
            const char *val = (strncmp(argv[i], "--tick-ms=", 10) == 0) ? argv[i] + 10 : argv[i] + 7;
            tick_interval_ms = atoi(val);
            if (tick_interval_ms <= 0) {
                return false;
            }
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strncmp(argv[i], "--deadlock=", 11) == 0) {
            const char *val = argv[i] + 11;
            if (strcmp(val, "prevention") == 0) {
                deadlock_strategy = "prevention";
            } else if (strcmp(val, "detection") == 0) {
                /* Deadlock detection not implemented; clearly document and fall back to prevention */
                fprintf(stderr, "Warning: deadlock detection not implemented; using prevention instead.\n");
                deadlock_strategy = "prevention";
            } else {
                print_usage(argv[0]);
                return false;
            }
        } else {
            print_usage(argv[0]);
            return false;
        }
    }

    return true;
}

/*
 * Thread-safe logger.
 */
void log_message(const char* format, ...) {

    if (!verbose) {
        return;
    }

    va_list args;

    va_start(args, format);

    vprintf(format, args);

    va_end(args);
}

/* Thread-safe printing used for execution log */
void print_log(const char* format, ...) {

    pthread_mutex_lock(&print_lock);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    fflush(stdout);

    pthread_mutex_unlock(&print_lock);
}