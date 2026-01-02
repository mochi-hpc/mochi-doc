/*
 * Work-stealing: Execution streams share pools and can steal work from each other
 * This improves load balancing when some execution streams finish their work early
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <abt.h>

#define NUM_XSTREAMS 4
#define NUM_THREADS 16

typedef struct {
    int thread_id;
} thread_arg_t;

void thread_func(void *arg)
{
    int thread_id = ((thread_arg_t *)arg)->thread_id;
    int xstream_rank;

    /* Get the rank of the execution stream running this ULT */
    ABT_xstream_self_rank(&xstream_rank);

    printf("ULT %2d executing on ES %d", thread_id, xstream_rank);

    /* Simulate varying work amounts */
    if (thread_id % 4 == 0) {
        printf(" (heavy work)");
        usleep(100000); /* 100ms */
    } else {
        printf(" (light work)");
        usleep(10000);  /* 10ms */
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int i, j;
    ABT_xstream xstreams[NUM_XSTREAMS];
    ABT_pool pools[NUM_XSTREAMS];
    ABT_sched scheds[NUM_XSTREAMS];
    ABT_thread threads[NUM_THREADS];
    thread_arg_t thread_args[NUM_THREADS];

    /* Initialize Argobots */
    ABT_init(argc, argv);

    printf("=== Work-Stealing Example ===\n");
    printf("Creating %d execution streams with shared pools\n\n", NUM_XSTREAMS);

    /* Create pools with work-stealing capability
       ABT_POOL_ACCESS_MPMC = Multiple Producers, Multiple Consumers
       This allows multiple execution streams to access the pool */
    for (i = 0; i < NUM_XSTREAMS; i++) {
        ABT_pool_create_basic(ABT_POOL_FIFO,           /* Pool kind: FIFO */
                              ABT_POOL_ACCESS_MPMC,    /* Access: thread-safe */
                              ABT_TRUE,                /* Automatic free */
                              &pools[i]);
    }

    /* Create schedulers that can access ALL pools
       Each scheduler can steal work from other pools when its own pool is empty */
    for (i = 0; i < NUM_XSTREAMS; i++) {
        ABT_pool *sched_pools = (ABT_pool *)malloc(sizeof(ABT_pool) * NUM_XSTREAMS);

        /* Pool priority order: own pool first, then others in round-robin */
        for (j = 0; j < NUM_XSTREAMS; j++) {
            sched_pools[j] = pools[(i + j) % NUM_XSTREAMS];
        }

        /* Create a scheduler with access to all pools */
        ABT_sched_create_basic(ABT_SCHED_DEFAULT,      /* Default scheduler */
                               NUM_XSTREAMS,            /* Number of pools */
                               sched_pools,             /* Array of pools */
                               ABT_SCHED_CONFIG_NULL,   /* Default config */
                               &scheds[i]);
        free(sched_pools);
    }

    /* Set up the primary execution stream with its scheduler */
    ABT_xstream_self(&xstreams[0]);
    ABT_xstream_set_main_sched(xstreams[0], scheds[0]);

    /* Create secondary execution streams with their schedulers */
    for (i = 1; i < NUM_XSTREAMS; i++) {
        ABT_xstream_create(scheds[i], &xstreams[i]);
    }

    /* Create ULTs and add them to pools */
    printf("Creating %d ULTs (some with heavy work)...\n\n", NUM_THREADS);
    for (i = 0; i < NUM_THREADS; i++) {
        int pool_id = i % NUM_XSTREAMS;
        thread_args[i].thread_id = i;

        /* ULTs are initially added to specific pools, but can be stolen
           by other execution streams if those streams run out of work */
        ABT_thread_create(pools[pool_id], thread_func, &thread_args[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all ULTs to complete */
    for (i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    /* Join and free secondary execution streams */
    for (i = 1; i < NUM_XSTREAMS; i++) {
        ABT_xstream_join(xstreams[i]);
        ABT_xstream_free(&xstreams[i]);
    }

    printf("\nAll ULTs completed\n");
    printf("Note: ULTs may have executed on different execution streams\n");
    printf("      due to work-stealing for better load balancing\n");

    /* Finalize Argobots */
    ABT_finalize();

    return 0;
}
