/*
 * Interoperability: Argobots ULTs and pthreads sharing mutex/cond
 * Critical for Mochi services that integrate with MPI or other pthread-based libraries
 */

#include <stdio.h>
#include <pthread.h>
#include <abt.h>

#define NUM_ULT_WORKERS 2
#define NUM_PTHREAD_WORKERS 2

typedef struct {
    int counter;
    ABT_mutex mutex;
    ABT_cond cond;
    int target;
} shared_data_t;

typedef struct {
    int worker_id;
    shared_data_t *shared;
} worker_arg_t;

void ult_worker(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;
    shared_data_t *shared = worker->shared;

    ABT_mutex_lock(shared->mutex);
    shared->counter++;
    printf("ULT worker %d: incremented counter to %d\n",
           worker->worker_id, shared->counter);

    /* Signal if target reached */
    if (shared->counter >= shared->target) {
        ABT_cond_broadcast(shared->cond);
    }
    ABT_mutex_unlock(shared->mutex);
}

void *pthread_worker(void *arg)
{
    worker_arg_t *worker = (worker_arg_t *)arg;
    shared_data_t *shared = worker->shared;

    /* pthreads can use Argobots mutex/cond */
    ABT_mutex_lock(shared->mutex);
    shared->counter++;
    printf("  pthread worker %d: incremented counter to %d\n",
           worker->worker_id, shared->counter);

    /* Signal if target reached */
    if (shared->counter >= shared->target) {
        ABT_cond_broadcast(shared->cond);
    }
    ABT_mutex_unlock(shared->mutex);

    return NULL;
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread ult_threads[NUM_ULT_WORKERS];
    pthread_t pthreads[NUM_PTHREAD_WORKERS];
    worker_arg_t ult_args[NUM_ULT_WORKERS];
    worker_arg_t pthread_args[NUM_PTHREAD_WORKERS];
    shared_data_t shared;

    ABT_init(argc, argv);

    printf("=== Pthread Interoperability ===\n");
    printf("Mixing Argobots ULTs and pthreads with shared synchronization\n\n");

    /* Initialize shared data */
    shared.counter = 0;
    shared.target = NUM_ULT_WORKERS + NUM_PTHREAD_WORKERS;
    ABT_mutex_create(&shared.mutex);
    ABT_cond_create(&shared.cond);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create ULT workers */
    for (int i = 0; i < NUM_ULT_WORKERS; i++) {
        ult_args[i].worker_id = i;
        ult_args[i].shared = &shared;
        ABT_thread_create(pool, ult_worker, &ult_args[i],
                          ABT_THREAD_ATTR_NULL, &ult_threads[i]);
    }

    /* Create pthread workers */
    for (int i = 0; i < NUM_PTHREAD_WORKERS; i++) {
        pthread_args[i].worker_id = i;
        pthread_args[i].shared = &shared;
        pthread_create(&pthreads[i], NULL, pthread_worker, &pthread_args[i]);
    }

    /* Wait for target in main thread */
    ABT_mutex_lock(shared.mutex);
    while (shared.counter < shared.target) {
        printf("Main: waiting for all workers (counter=%d/%d)\n",
               shared.counter, shared.target);
        ABT_cond_wait(shared.cond, shared.mutex);
    }
    ABT_mutex_unlock(shared.mutex);

    printf("\nAll workers completed! Counter: %d\n", shared.counter);

    /* Cleanup */
    for (int i = 0; i < NUM_ULT_WORKERS; i++) {
        ABT_thread_free(&ult_threads[i]);
    }
    for (int i = 0; i < NUM_PTHREAD_WORKERS; i++) {
        pthread_join(pthreads[i], NULL);
    }

    ABT_cond_free(&shared.cond);
    ABT_mutex_free(&shared.mutex);

    printf("Argobots mutex/cond worked with both ULTs and pthreads\n");

    ABT_finalize();
    return 0;
}
