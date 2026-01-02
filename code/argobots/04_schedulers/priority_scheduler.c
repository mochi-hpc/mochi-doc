/*
 * PRIO scheduler example: Priority-based scheduling
 */

#include <stdio.h>
#include <abt.h>

#define NUM_HIGH 4
#define NUM_LOW 4

void work_func(void *arg)
{
    char *priority = (char *)arg;
    int rank;
    ABT_xstream_self_rank(&rank);
    printf("Thread (%s priority) on ES %d\n", priority, rank);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool high_pool, low_pool;
    ABT_sched sched;
    ABT_thread high_threads[NUM_HIGH], low_threads[NUM_LOW];
    ABT_pool pools[2];

    ABT_init(argc, argv);

    printf("=== PRIO Scheduler Example ===\n\n");

    /* Create high and low priority pools */
    ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                          ABT_TRUE, &high_pool);
    ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                          ABT_TRUE, &low_pool);

    /* Priority order: high_pool first, then low_pool */
    pools[0] = high_pool;
    pools[1] = low_pool;

    /* Create PRIO scheduler with priority pools */
    ABT_sched_create_basic(ABT_SCHED_PRIO, 2, pools,
                           ABT_SCHED_CONFIG_NULL, &sched);

    ABT_xstream_self(&xstream);
    ABT_xstream_set_main_sched(xstream, sched);

    /* Create low priority threads first */
    for (int i = 0; i < NUM_LOW; i++) {
        ABT_thread_create(low_pool, work_func, "LOW",
                          ABT_THREAD_ATTR_NULL, &low_threads[i]);
    }

    /* Create high priority threads */
    for (int i = 0; i < NUM_HIGH; i++) {
        ABT_thread_create(high_pool, work_func, "HIGH",
                          ABT_THREAD_ATTR_NULL, &high_threads[i]);
    }

    /* Wait for all threads */
    for (int i = 0; i < NUM_HIGH; i++) {
        ABT_thread_free(&high_threads[i]);
    }
    for (int i = 0; i < NUM_LOW; i++) {
        ABT_thread_free(&low_threads[i]);
    }

    printf("\nPRIO scheduler: High priority pool drained before low priority\n");

    ABT_finalize();
    return 0;
}
