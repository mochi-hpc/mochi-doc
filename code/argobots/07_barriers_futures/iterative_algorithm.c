/*
 * Iterative algorithm with barrier reinitialization
 * Demonstrates reusing barriers for multiple algorithm runs
 */

#include <stdio.h>
#include <abt.h>

#define NUM_THREADS 4
#define ITERATIONS_PER_RUN 5
#define NUM_RUNS 3

typedef struct {
    int thread_id;
    int iteration;
    ABT_barrier barrier;
} work_arg_t;

void iterative_work(void *arg)
{
    work_arg_t *work = (work_arg_t *)arg;
    int id = work->thread_id;

    for (int i = 0; i < ITERATIONS_PER_RUN; i++) {
        /* Do some work */
        int rank;
        ABT_xstream_self_rank(&rank);
        printf("  Thread %d (ES %d), iteration %d\n", id, rank, i);

        /* Barrier: Synchronize all threads at end of iteration */
        ABT_barrier_wait(work->barrier);

        /* Master thread prints completion message */
        if (id == 0) {
            printf("    -> Iteration %d completed by all threads\n", i);
        }

        /* Another barrier to ensure message is printed before next iteration */
        ABT_barrier_wait(work->barrier);
    }
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_THREADS];
    ABT_barrier barrier;
    work_arg_t work_args[NUM_THREADS];

    ABT_init(argc, argv);

    printf("=== Iterative Algorithm with Barrier Reuse ===\n");
    printf("Running %d separate runs with %d iterations each\n\n",
           NUM_RUNS, ITERATIONS_PER_RUN);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create barrier once */
    ABT_barrier_create(NUM_THREADS, &barrier);

    for (int run = 0; run < NUM_RUNS; run++) {
        printf("Run %d:\n", run);

        /* Create threads for this run */
        for (int i = 0; i < NUM_THREADS; i++) {
            work_args[i].thread_id = i;
            work_args[i].iteration = run;
            work_args[i].barrier = barrier;

            ABT_thread_create(pool, iterative_work, &work_args[i],
                              ABT_THREAD_ATTR_NULL, &threads[i]);
        }

        /* Wait for all threads to complete this run */
        for (int i = 0; i < NUM_THREADS; i++) {
            ABT_thread_free(&threads[i]);
        }

        printf("Run %d completed\n\n", run);

        /* Reinitialize barrier for next run */
        if (run < NUM_RUNS - 1) {
            ABT_barrier_reinit(barrier, NUM_THREADS);
        }
    }

    /* Free barrier after all runs */
    ABT_barrier_free(&barrier);

    printf("All runs completed. Barrier was reused %d times via reinit\n", NUM_RUNS);

    ABT_finalize();
    return 0;
}
