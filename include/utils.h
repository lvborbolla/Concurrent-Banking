#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

extern const char* accounts_file;
extern const char* trace_file;
extern const char* deadlock_strategy;
extern int tick_interval_ms;

/* CLI parsing */
bool parse_arguments(int argc, char* argv[]);

/* Logging */
void log_message(const char* format, ...);

#endif