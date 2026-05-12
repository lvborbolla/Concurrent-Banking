#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

const char* accounts_file = NULL;
const char* trace_file = NULL;
const char* deadlock_strategy = "prevention";
int tick_interval_ms = 100;

static void print_usage(const char* program_name) {

    fprintf(stderr,
            "Usage: %s --accounts=FILE --trace=FILE [--tick=MS] [--deadlock=prevention]\n",
            program_name);
}

/*
 * Parse command-line arguments.
 */
bool parse_arguments(int argc, char* argv[]) {

    accounts_file = NULL;
    trace_file = NULL;
    deadlock_strategy = "prevention";
    tick_interval_ms = 100;

    for (int i = 1; i < argc; i++) {

        if (strncmp(argv[i], "--accounts=", 11) == 0) {
            accounts_file = argv[i] + 11;
        } else if (strncmp(argv[i], "--trace=", 8) == 0) {
            trace_file = argv[i] + 8;
        } else if (strncmp(argv[i], "--tick=", 7) == 0) {
            tick_interval_ms = atoi(argv[i] + 7);
            if (tick_interval_ms <= 0) {
                return false;
            }
        } else if (strncmp(argv[i], "--deadlock=", 11) == 0) {
            deadlock_strategy = argv[i] + 11;
            if (strcmp(deadlock_strategy, "prevention") != 0) {
                return false;
            }
        } else {
            print_usage(argv[0]);
            return false;
        }
    }

    if (accounts_file == NULL || trace_file == NULL) {
        print_usage(argv[0]);
        return false;
    }

    return true;
}

/*
 * Thread-safe logger.
 */
void log_message(const char* format, ...) {

    va_list args;

    va_start(args, format);

    vprintf(format, args);

    va_end(args);
}