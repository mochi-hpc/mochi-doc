/*
 * Work-stealing scheduler (RANDWS) example
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <abt.h>

#define NUM_XSTREAMS 4
#define NUM_THREADS 16

void work_func(void *arg)
{
    int id = *(int *)arg;
    int rank;
    ABT_xstream_self_rank(&rank);

    printf("Thread %2d started on ES %d", id, rank);

    /* Simulate varying workload */
    if (id % 4 == 0) {
        printf(" (heavy)");
        usleep(50000);
    } else {
        printf(" (light)");
        usleep(5000);
    }

    ABT_xstream_self_rank(&rank);
    printf(" -> finished on ES %d\n", rank);
}

int main(int argc, char **argv)
{
    ABT_xstream xstreams[NUM_XSTREAMS];
    ABT_pool pools[NUM_XSTREAMS];
    ABT_sched scheds[NUM_XSTREAMS];
    ABT_thread threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    ABT_init(argc, argv);

    printf("=== Work-Stealing (RANDWS) Scheduler Example ===\n\n");

    /* Create pools */
    for (int i = 0; i < NUM_XSTREAMS; i++) {
        ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                              ABT_TRUE, &pools[i]);
    }

    /* Create schedulers with work-stealing capability */
    for (int i = 0; i < NUM_XSTREAMS; i++) {
        ABT_pool *sched_pools = malloc(sizeof(ABT_pool) * NUM_XSTREAMS);

        /* Each scheduler can access all pools (own pool first) */
        for (int j = 0; j < NUM_XSTREAMS; j++) {
            sched_pools[j] = pools[(i + j) % NUM_XSTREAMS];
        }

        ABT_sched_create_basic(ABT_SCHED_RANDWS, NUM_XSTREAMS,
                               sched_pools, ABT_SCHED_CONFIG_NULL, &scheds[i]);
        free(sched_pools);
    }

    /* Setup execution streams */
    ABT_xstream_self(&xstreams[0]);
    ABT_xstream_set_main_sched(xstreams[0], scheds[0]);

    for (int i = 1; i < NUM_XSTREAMS; i++) {
        ABT_xstream_create(scheds[i], &xstreams[i]);
    }

    /* Create threads distributed across pools */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        int pool_id = i % NUM_XSTREAMS;
        ABT_thread_create(pools[pool_id], work_func, &thread_ids[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    /* Cleanup */
    for (int i = 1; i < NUM_XSTREAMS; i++) {
        ABT_xstream_join(xstreams[i]);
        ABT_xstream_free(&xstreams[i]);
    }

    printf("\nRANDWS scheduler: Work stolen for load balancing\n");

    ABT_finalize();
    return 0;
}
