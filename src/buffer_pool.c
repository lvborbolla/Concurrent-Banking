#include "buffer_pool.h"

BufferPool buffer_pool;

/*
 * Initialize pool.
 */
void init_buffer_pool(BufferPool* pool) {

    sem_init(&pool->empty_slots,
             0,
             BUFFER_POOL_SIZE);

    sem_init(&pool->full_slots,
             0,
             0);

    pthread_mutex_init(&pool->pool_lock,
                       NULL);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {

        pool->slots[i].account_id = -1;
        pool->slots[i].in_use = false;
    }
}

/*
 * Load account into buffer.
 */
void load_account(BufferPool* pool,
                  int account_id) {

    sem_wait(&pool->empty_slots);

    pthread_mutex_lock(&pool->pool_lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {

        if (!pool->slots[i].in_use) {

            pool->slots[i].in_use = true;
            pool->slots[i].account_id = account_id;

            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    sem_post(&pool->full_slots);
}

/*
 * Remove account from buffer.
 */
void unload_account(BufferPool* pool,
                    int account_id) {

    sem_wait(&pool->full_slots);

    pthread_mutex_lock(&pool->pool_lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {

        if (pool->slots[i].in_use &&
            pool->slots[i].account_id == account_id) {

            pool->slots[i].in_use = false;
            pool->slots[i].account_id = -1;

            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    sem_post(&pool->empty_slots);
}