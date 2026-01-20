/*
 * Parallel reduction with futures
 * Demonstrates multiple-producer-single-consumer synchronization
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_WORKERS 4
#define ARRAY_SIZE 1000

typedef struct {
    int worker_id;
    int *data;
    int start;
    int end;
    ABT_future result_future;
} worker_arg_t;

/* Storage for partial results - must persist beyond worker lifetime */
int partial_results[NUM_WORKERS];

/* Callback invoked when all workers complete
 * args[i] contains pointer passed by i-th worker's ABT_future_set()
 */
void reduction_callback(void **args)
{
    int total = 0;
    printf("\n=== Callback invoked - all workers done ===\n");

    /* Sum up all partial results */
    for (int i = 0; i < NUM_WORKERS; i++) {
        int *partial = (int *)args[i];
        printf("  Worker %d contributed: %d\n", i, *partial);
        total += *partial;
    }

    printf("  Total sum: %d\n", total);
}

void worker_func(void *arg)
{
    worker_arg_t *warg = (worker_arg_t *)arg;
    int sum = 0;

    /* Compute partial sum for assigned range */
    for (int i = warg->start; i < warg->end; i++) {
        sum += warg->data[i];
    }

    /* Store result in persistent location */
    partial_results[warg->worker_id] = sum;

    printf("Worker %d computed partial sum: %d (range %d-%d)\n",
           warg->worker_id, sum, warg->start, warg->end);

    /* Set this worker's compartment in the future
     * This is the ONLY way to pass the value - via the callback
     */
    ABT_future_set(warg->result_future, &partial_results[warg->worker_id]);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread workers[NUM_WORKERS];
    ABT_future result_future;
    worker_arg_t worker_args[NUM_WORKERS];

    /* Initialize test data */
    int *data = (int *)malloc(ARRAY_SIZE * sizeof(int));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i + 1;  /* 1, 2, 3, ... */
    }

    ABT_init(argc, argv);

    printf("=== Multiple-Producer-Single-Consumer with Futures ===\n");
    printf("Computing sum of array with %d workers\n\n", NUM_WORKERS);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create future with NUM_WORKERS compartments
     * One compartment per worker (multiple producers)
     * Main thread is the single consumer
     * Callback will be invoked when ALL workers complete
     */
    ABT_future_create(NUM_WORKERS, reduction_callback, &result_future);

    /* Launch workers */
    int chunk_size = ARRAY_SIZE / NUM_WORKERS;
    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_args[i].worker_id = i;
        worker_args[i].data = data;
        worker_args[i].start = i * chunk_size;
        worker_args[i].end = (i == NUM_WORKERS - 1) ? ARRAY_SIZE : (i + 1) * chunk_size;
        worker_args[i].result_future = result_future;

        ABT_thread_create(pool, worker_func, &worker_args[i],
                          ABT_THREAD_ATTR_NULL, &workers[i]);
    }

    /* Main thread waits for ALL workers to complete
     * This blocks until all NUM_WORKERS compartments are set
     * Then the callback is invoked automatically
     */
    printf("\nMain thread waiting for all workers...\n");
    ABT_future_wait(result_future);
    printf("Main thread unblocked - all workers completed!\n");

    /* Note: There is NO ABT_future_get() function!
     * The only way to retrieve values is via the callback.
     * If you need the result in main, the callback must store it somewhere.
     */

    /* Verify against sequential computation */
    int expected = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected += data[i];
    }
    printf("\nExpected sum (sequential): %d\n", expected);

    /* Wait for workers to join */
    for (int i = 0; i < NUM_WORKERS; i++) {
        ABT_thread_free(&workers[i]);
    }

    ABT_future_free(&result_future);
    free(data);

    printf("\nKey pattern demonstrated:\n");
    printf("- Multiple producers (workers) each set one compartment\n");
    printf("- Single consumer (main) waits for all compartments\n");
    printf("- Callback receives all values when complete\n");
    printf("- This is between eventual (1-to-1) and barrier (N-to-N)\n");

    ABT_finalize();
    return 0;
}
