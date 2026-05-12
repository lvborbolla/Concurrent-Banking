#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>
#include <stdbool.h>

/* Global simulation clock */
extern volatile int global_tick;

/* Synchronization */
extern pthread_mutex_t tick_lock;
extern pthread_cond_t tick_changed;

/* Control flags */
extern bool simulation_running;

/* Timer thread */
void* timer_thread(void* arg);

/* Wait until target tick */
void wait_until_tick(int target_tick);

/* Snapshot current tick under tick_lock */
int get_current_tick(void);

#endif