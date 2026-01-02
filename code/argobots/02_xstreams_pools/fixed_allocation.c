/*
 * Fixed allocation: Each execution stream has its own private pool
 * ULTs are statically assigned to pools and cannot migrate
 */

#include <stdio.h>
#include <stdlib.h>
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

    printf("ULT %2d executing on ES %d (fixed allocation)\n",
           thread_id, xstream_rank);
}

int main(int argc, char **argv)
{
    int i;
    ABT_xstream xstreams[NUM_XSTREAMS];
    ABT_pool pools[NUM_XSTREAMS];
    ABT_thread threads[NUM_THREADS];
    thread_arg_t thread_args[NUM_THREADS];

    /* Initialize Argobots */
    ABT_init(argc, argv);

    printf("=== Fixed Allocation Example ===\n");
    printf("Creating %d execution streams with private pools\n\n", NUM_XSTREAMS);

    /* Get the primary execution stream */
    ABT_xstream_self(&xstreams[0]);

    /* Get the primary execution stream's default pool */
    ABT_xstream_get_main_pools(xstreams[0], 1, &pools[0]);

    /* Create additional execution streams (each gets its own pool automatically) */
    for (i = 1; i < NUM_XSTREAMS; i++) {
        ABT_xstream_create(ABT_SCHED_NULL, &xstreams[i]);
        /* Get the default pool for this execution stream */
        ABT_xstream_get_main_pools(xstreams[i], 1, &pools[i]);
    }

    /* Create ULTs and assign them to pools in round-robin fashion */
    for (i = 0; i < NUM_THREADS; i++) {
        int pool_id = i % NUM_XSTREAMS;
        thread_args[i].thread_id = i;

        /* Each ULT is added to a specific pool and will only execute on
           that pool's execution stream */
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
    printf("Note: Each ULT executed only on its assigned execution stream\n");

    /* Finalize Argobots */
    ABT_finalize();

    return 0;
}
