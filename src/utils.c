#include <stdio.h>
#include <stdarg.h>

#include "utils.h"

/*
 * Parse command-line arguments.
 */
bool parse_arguments(int argc, char* argv[]) {

    /* TODO */

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