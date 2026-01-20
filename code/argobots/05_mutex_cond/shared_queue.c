/*
 * Thread-safe queue implementation with mutexes using static initialization
 * Demonstrates ABT_mutex_memory to avoid heap allocation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abt.h>

#define QUEUE_SIZE 10
#define NUM_WORKERS 4
#define ITEMS_PER_WORKER 5

typedef struct {
    int *data;
    int capacity;
    int size;
    int head;
    int tail;
    ABT_mutex_memory mutex_mem;  /* Static mutex memory */
} thread_safe_queue_t;

void queue_init(thread_safe_queue_t *q, int capacity)
{
    q->data = malloc(sizeof(int) * capacity);
    q->capacity = capacity;
    q->size = 0;
    q->head = 0;
    q->tail = 0;
    /* Initialize mutex memory
     * This is equivalent to setting it to ABT_MUTEX_INITIALIZER */
    memset(&q->mutex_mem, 0, sizeof(q->mutex_mem));
}

int queue_push(thread_safe_queue_t *q, int value)
{
    /* Convert mutex_memory to ABT_mutex handle */
    ABT_mutex mutex = ABT_MUTEX_MEMORY_GET_HANDLE(&q->mutex_mem);

    ABT_mutex_lock(mutex);

    if (q->size == q->capacity) {
        ABT_mutex_unlock(mutex);
        return -1;  /* Queue full */
    }

    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;

    ABT_mutex_unlock(mutex);
    return 0;
}

int queue_pop(thread_safe_queue_t *q, int *value)
{
    /* Convert mutex_memory to ABT_mutex handle */
    ABT_mutex mutex = ABT_MUTEX_MEMORY_GET_HANDLE(&q->mutex_mem);

    ABT_mutex_lock(mutex);

    if (q->size == 0) {
        ABT_mutex_unlock(mutex);
        return -1;  /* Queue empty */
    }

    *value = q->data[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;

    ABT_mutex_unlock(mutex);
    return 0;
}

void queue_destroy(thread_safe_queue_t *q)
{
    /* No need to free mutex_mem - no heap allocation */
    free(q->data);
}

typedef struct {
    int worker_id;
    thread_safe_queue_t *queue;
} worker_arg_t;

void worker_thread(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;

    /* Each worker pushes some items */
    for (int i = 0; i < ITEMS_PER_WORKER; i++) {
        int item = worker->worker_id * 100 + i;

        while (queue_push(worker->queue, item) != 0) {
            /* Queue full, yield and retry */
            ABT_thread_yield();
        }

        printf("Worker %d: pushed %d\n", worker->worker_id, item);
    }

    /* Then pop some items */
    for (int i = 0; i < ITEMS_PER_WORKER; i++) {
        int item;

        while (queue_pop(worker->queue, &item) != 0) {
            /* Queue empty, yield and retry */
            ABT_thread_yield();
        }

        printf("  Worker %d: popped %d\n", worker->worker_id, item);
    }
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_WORKERS];
    worker_arg_t worker_args[NUM_WORKERS];
    thread_safe_queue_t queue;

    ABT_init(argc, argv);

    printf("=== Thread-Safe Queue (Static Mutex) ===\n");
    printf("Workers: %d, Queue capacity: %d\n", NUM_WORKERS, QUEUE_SIZE);
    printf("Using ABT_mutex_memory for zero-overhead initialization\n\n");

    queue_init(&queue, QUEUE_SIZE);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create worker threads */
    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_args[i].worker_id = i;
        worker_args[i].queue = &queue;
        ABT_thread_create(pool, worker_thread, &worker_args[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all workers */
    for (int i = 0; i < NUM_WORKERS; i++) {
        ABT_thread_free(&threads[i]);
    }

    queue_destroy(&queue);

    printf("\nAll operations completed safely with mutex protection\n");
    printf("No heap allocation needed for mutex (ABT_mutex_memory)\n");

    ABT_finalize();
    return 0;
}
