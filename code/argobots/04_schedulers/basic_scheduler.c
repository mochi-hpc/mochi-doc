/*
 * BASIC scheduler example: Simple round-robin scheduling
 */

#include <stdio.h>
#include <abt.h>

#define NUM_THREADS 8

void thread_func(void *arg)
{
    int id = *(int *)arg;
    int rank;
    ABT_xstream_self_rank(&rank);
    printf("Thread %d on ES %d (BASIC scheduler)\n", id, rank);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_sched sched;
    ABT_thread threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    ABT_init(argc, argv);

    printf("=== BASIC Scheduler Example ===\n\n");

    /* Create a pool */
    ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                          ABT_TRUE, &pool);

    /* Create BASIC scheduler */
    ABT_sched_create_basic(ABT_SCHED_BASIC, 1, &pool,
                           ABT_SCHED_CONFIG_NULL, &sched);

    /* Set scheduler on primary xstream */
    ABT_xstream_self(&xstream);
    ABT_xstream_set_main_sched(xstream, sched);

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        ABT_thread_create(pool, thread_func, &thread_ids[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for completion */
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    printf("\nBASIC scheduler: simple FIFO execution\n");

    ABT_finalize();
    return 0;
}
