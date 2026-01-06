/*
 * Yield-based synchronization: Cooperative multitasking
 */

#include <stdio.h>
#include <abt.h>

#define NUM_WORKERS 3
#define ITERATIONS 5

typedef struct {
    int worker_id;
    int *shared_counter;
    ABT_mutex mutex;
} worker_arg_t;

void cooperative_worker(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        /* Try to acquire lock */
        ABT_mutex_lock(worker->mutex);

        (*worker->shared_counter)++;
        printf("Worker %d: iteration %d, counter = %d\n",
               worker->worker_id, i, *worker->shared_counter);

        ABT_mutex_unlock(worker->mutex);

        /* Voluntarily yield to let other workers run */
        printf("  Worker %d yielding...\n", worker->worker_id);
        ABT_self_yield();
    }

    printf("Worker %d completed\n", worker->worker_id);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread workers[NUM_WORKERS];
    ABT_mutex mutex;
    worker_arg_t worker_args[NUM_WORKERS];
    int shared_counter = 0;

    ABT_init(argc, argv);

    printf("=== Yield-Based Cooperative Scheduling ===\n\n");

    ABT_mutex_create(&mutex);
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create workers */
    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_args[i].worker_id = i;
        worker_args[i].shared_counter = &shared_counter;
        worker_args[i].mutex = mutex;

        ABT_thread_create(pool, cooperative_worker, &worker_args[i],
                          ABT_THREAD_ATTR_NULL, &workers[i]);
    }

    /* Wait for all */
    for (int i = 0; i < NUM_WORKERS; i++) {
        ABT_thread_free(&workers[i]);
    }

    ABT_mutex_free(&mutex);

    printf("\nYielding enabled fair cooperative scheduling\n");
    printf("Final counter: %d\n", shared_counter);

    ABT_finalize();
    return 0;
}
