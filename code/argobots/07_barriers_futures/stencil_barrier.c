/*
 * Stencil computation with barriers
 * Demonstrates bulk-synchronous parallel pattern
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 16
#define NUM_ITERATIONS 3

typedef struct {
    int thread_id;
    int num_threads;
    double *array;
    double *temp;
    ABT_barrier barrier;
} work_arg_t;

void stencil_worker(void *arg)
{
    work_arg_t *work = (work_arg_t *)arg;
    int id = work->thread_id;
    int num = work->num_threads;
    int chunk_size = ARRAY_SIZE / num;
    int start = id * chunk_size;
    int end = (id == num - 1) ? ARRAY_SIZE : start + chunk_size;

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        /* Compute stencil: temp[i] = average of array[i-1], array[i], array[i+1] */
        for (int i = start; i < end; i++) {
            int left = (i == 0) ? 0 : i - 1;
            int right = (i == ARRAY_SIZE - 1) ? ARRAY_SIZE - 1 : i + 1;
            work->temp[i] = (work->array[left] + work->array[i] + work->array[right]) / 3.0;
        }

        /* Barrier: Wait for all threads to finish computation */
        ABT_barrier_wait(work->barrier);

        /* Copy temp back to array */
        for (int i = start; i < end; i++) {
            work->array[i] = work->temp[i];
        }

        /* Barrier: Wait for all threads to finish copying */
        ABT_barrier_wait(work->barrier);

        if (id == 0) {
            printf("Iteration %d completed\n", iter);
        }
    }
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_THREADS];
    ABT_barrier barrier;
    work_arg_t work_args[NUM_THREADS];
    double array[ARRAY_SIZE];
    double temp[ARRAY_SIZE];

    ABT_init(argc, argv);

    printf("=== Stencil Computation with Barriers ===\n");
    printf("Array size: %d, Threads: %d, Iterations: %d\n\n",
           ARRAY_SIZE, NUM_THREADS, NUM_ITERATIONS);

    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * 10.0;
    }

    printf("Initial array:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%.1f ", array[i]);
    }
    printf("\n\n");

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create barrier for NUM_THREADS threads */
    ABT_barrier_create(NUM_THREADS, &barrier);

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        work_args[i].thread_id = i;
        work_args[i].num_threads = NUM_THREADS;
        work_args[i].array = array;
        work_args[i].temp = temp;
        work_args[i].barrier = barrier;

        ABT_thread_create(pool, stencil_worker, &work_args[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    printf("\nFinal array:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%.1f ", array[i]);
    }
    printf("\n\n");

    /* Free barrier */
    ABT_barrier_free(&barrier);

    printf("Barriers ensured all threads completed each phase before proceeding\n");

    ABT_finalize();
    return 0;
}
