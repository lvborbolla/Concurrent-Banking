#include <unistd.h>

#include "timer.h"
#include "utils.h"

volatile int global_tick = 0;

pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t tick_changed = PTHREAD_COND_INITIALIZER;

bool simulation_running = true;

/*
 * Global timer thread.
 */
void* timer_thread(void* arg) {

    int tick_ms = *((int*)arg);

    while (simulation_running) {

        usleep(tick_ms * 1000);

        pthread_mutex_lock(&tick_lock);

        global_tick++;

        /* Print tick header for execution log */
        print_log("\nTick %d:\n", global_tick);

        pthread_cond_broadcast(&tick_changed);

        pthread_mutex_unlock(&tick_lock);
    }

    return NULL;
}

/*
 * Block until target tick.
 */
void wait_until_tick(int target_tick) {

    pthread_mutex_lock(&tick_lock);

    while (global_tick < target_tick) {

        pthread_cond_wait(&tick_changed,
                          &tick_lock);
    }

    pthread_mutex_unlock(&tick_lock);
}