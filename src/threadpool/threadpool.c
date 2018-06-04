//
// Created by jad on 01/06/18.
//

#include "threadpool.h"

work_queue_t *init_queue(uint16_t max_items_) {

    //printf("Initialising queue. max length: %d\n", max_items_);

    work_queue_t *work_queue = (work_queue_t*)malloc(sizeof(work_queue_t));
    if (work_queue == NULL) {
        return NULL;
    }

    work_queue->max_items = max_items_;
    work_queue->items = malloc(sizeof(task_t) * max_items_);
    work_queue->head = 0;
    work_queue->tail = 0;
    work_queue->item_count = 0;
    work_queue->space_left = work_queue->max_items;

    work_queue->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (pthread_mutex_init(work_queue->lock, NULL) != 0) {
        return NULL;
    }
    if (work_queue->items == NULL) {
        return NULL;
    }

    return work_queue;

}

thread_pool_t *init_thread_pool(uint16_t nthreads_, work_queue_t *work_queue) {

    //printf("Initialising thread pool. nthreads: %d\n", nthreads_);

    thread_pool_t *thread_pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    if (thread_pool == NULL) {
        return NULL;
    }

    thread_pool->nthreads = nthreads_;
    thread_pool->threads = malloc(sizeof(pthread_t) * nthreads_);
    if (thread_pool->threads == NULL) {
        return NULL;
    }

    thread_pool->status = pool_ready;

    //printf("creating pthread instances\n");
    worker_args_t *worker_args;
    pthread_t *tmp;
    int i;
    for (i = 0; i < nthreads_; ++i) {
        worker_args = (worker_args_t*)malloc(sizeof(worker_args_t));
        worker_args->worker_id = i;
        worker_args->parent_pool = thread_pool;
        worker_args->work_queue = work_queue;
        tmp = &thread_pool->threads[i];
        if (pthread_create(tmp, NULL, &worker_thread, worker_args) != 0) {
            free_thread_pool(thread_pool);
            return NULL;
        }
    }

    thread_pool->status = pool_running;
    return thread_pool;

}

void free_queue(work_queue_t *queue) {
    if ( queue != NULL ) {
        free(queue->items);
        pthread_mutex_lock(queue->lock);
        pthread_mutex_destroy(queue->lock);
        free(queue);
    }
}

void free_thread_pool(thread_pool_t *thread_pool) {
    int i;
    pthread_t *threadptr;
    if ( thread_pool != NULL ) {
        for (i = 0; i < thread_pool->nthreads; ++i) {
            threadptr = &thread_pool->threads[i];
            if (threadptr != NULL) {
                pthread_join(*threadptr, NULL);
//                free(threadptr);
                printf("Joined thread: %d\n", i);
            }
        }

        free(thread_pool->threads);
        free(thread_pool);
    }
}

void *worker_thread(void *worker_args) {
    worker_args_t *args = (worker_args_t*)worker_args;
    task_t *task;

    printf("[thread %d] started\n", args->worker_id);



    while(1) {

        // acquire lock
        pthread_mutex_lock(args->work_queue->lock);

        // check if we are supposed to run
        if (args->parent_pool->status == pool_running) {
            // get task from queue
            if (args->work_queue->item_count > 0) {
                task = args->work_queue->items[args->work_queue->tail];
                //printf("[thread %d] task obtained!\n", args->worker_id);

                if (args->work_queue->tail == (args->work_queue->max_items - 1)) {
                    args->work_queue->tail = 0;
                } else {
                    ++args->work_queue->tail;
                }

                --args->work_queue->item_count;
                ++args->work_queue->space_left;
            } else {
                printf("[thread %d] waiting for more jobs in pool\n", args->worker_id);
                //usleep(1);
                task = NULL;
            }

        } else if (args->parent_pool->status == pool_stopping) {
            printf("[thread %d] noticed pool is over\n", args->worker_id);
            break;
        } else {
            usleep(1);
            printf("[thread %d] waiting for pool to run or stop\n", args->worker_id);
            task = NULL;
        }

        // release the lock
        pthread_mutex_unlock(args->work_queue->lock);

        // do work!
        if (task != NULL) {
            float start_time = (float) clock() / CLOCKS_PER_SEC;
//            printf("[thread %d] task started!\n", args->worker_id);

            (task->function_ptr)(task->function_args);
            float stop_time = (float) clock() / CLOCKS_PER_SEC;
//            printf("[thread %d] task finished in %f seconds!\n", args->worker_id, (stop_time-start_time));
            free(task);
        }

        if (args->parent_pool->status == pool_stopping) {
            return NULL;
        }


    } // end loop

    printf("[thread %d] thread stopping!\n", args->worker_id);

//    free(args);
//    args = NULL; // kill the dangling pointer
    return NULL;
}

int add_task(work_queue_t *queue, task_t *task) {
    pthread_mutex_lock(queue->lock);

    if (queue->space_left > 0) {
        queue->items[queue->head] = task;
        ++queue->item_count;
        --queue->space_left;

        if (queue->head == (queue->max_items - 1)) {
            queue->head = 0;
        } else {
            ++queue->head;
        }

    } else {
        pthread_mutex_unlock(queue->lock);
        return 1;
    }

    pthread_mutex_unlock(queue->lock);
    return 0;

}
