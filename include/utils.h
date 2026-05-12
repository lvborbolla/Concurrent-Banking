#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

/* CLI parsing */
bool parse_arguments(int argc, char* argv[]);

/* Logging */
void log_message(const char* format, ...);

#endif