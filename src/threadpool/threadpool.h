//
// Created by jad on 01/06/18.
//

#ifndef DWCPNC_THREADPOOL_H
#define DWCPNC_THREADPOOL_H

#include <pthread.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

typedef enum {
    pool_ready    = 0,
    pool_running  = 1,
    pool_stopping = 2,
    pool_stopped  = 3
} pool_status_t;

typedef struct {
    void (*function_ptr)(void*);
    void *function_args;
} task_t;

typedef struct {
    uint16_t max_items;
    task_t **items;
    uint16_t head;
    uint16_t tail;
    uint16_t item_count;
    uint16_t space_left;
    pthread_mutex_t *lock;
} work_queue_t;

typedef struct {
    uint16_t nthreads;
    pthread_t *threads;
    pool_status_t status;
    pthread_mutex_t *lock;
} thread_pool_t;

typedef struct {
    int worker_id;
    thread_pool_t *parent_pool;
    work_queue_t *work_queue;
} worker_args_t;


work_queue_t *init_queue(uint16_t max_items_);
thread_pool_t *init_thread_pool(uint16_t nthreads_, work_queue_t *work_queue);

void free_queue(work_queue_t *queue);
void free_thread_pool(thread_pool_t *thread_pool);

void *worker_thread(void *worker_args);
int add_task(work_queue_t *queue, task_t *task);


#endif //DWCPNC_THREADPOOL_H
