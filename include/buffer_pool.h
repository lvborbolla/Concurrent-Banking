#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_POOL_SIZE 5

/*
 * Buffer slot structure.
 */
typedef struct {
    int account_id;
    bool in_use;
} BufferSlot;

/*
 * Buffer pool structure.
 */
typedef struct {
    BufferSlot slots[BUFFER_POOL_SIZE];

    sem_t empty_slots;
    sem_t full_slots;

    pthread_mutex_t pool_lock;

} BufferPool;

/* Global pool */
extern BufferPool buffer_pool;
extern unsigned long buffer_load_count;
extern unsigned long buffer_unload_count;
extern int buffer_current_occupancy;

/* Initialization */
void init_buffer_pool(BufferPool* pool);

/* Load/unload */
void load_account(BufferPool* pool, int account_id);

void unload_account(BufferPool* pool, int account_id);

#endif