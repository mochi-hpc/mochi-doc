/*
 * Recursive Fibonacci with work-stealing schedulers
 * Demonstrates divide-and-conquer parallelism with dynamic load balancing
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_XSTREAMS 4
#define FIB_N 20

ABT_pool *pools;
int num_pools;

typedef struct {
    int n;
    int result;
} fib_arg_t;

void fibonacci_ult(void *arg)
{
    fib_arg_t *fib = (fib_arg_t *)arg;
    int n = fib->n;

    if (n <= 2) {
        fib->result = 1;
        return;
    }

    /* Create child tasks */
    fib_arg_t child1 = {n - 1, 0};
    fib_arg_t child2 = {n - 2, 0};

    int rank;
    ABT_xstream_self_rank(&rank);
    ABT_pool target_pool = pools[rank % num_pools];

    ABT_thread thread1;
    ABT_thread_create(target_pool, fibonacci_ult, &child1,
                      ABT_THREAD_ATTR_NULL, &thread1);

    /* Calculate child2 directly (no need to create another ULT) */
    fibonacci_ult(&child2);

    /* Wait for child1 */
    ABT_thread_free(&thread1);

    fib->result = child1.result + child2.result;
}

int main(int argc, char **argv)
{
    ABT_xstream xstreams[NUM_XSTREAMS];
    ABT_pool local_pools[NUM_XSTREAMS];
    ABT_sched scheds[NUM_XSTREAMS];
    fib_arg_t fib_task = {FIB_N, 0};

    ABT_init(argc, argv);

    printf("=== Fibonacci with Work-Stealing ===\n");
    printf("Computing fib(%d) with %d execution streams\n\n", FIB_N, NUM_XSTREAMS);

    num_pools = NUM_XSTREAMS;
    pools = local_pools;

    /* Create pools */
    for (int i = 0; i < NUM_XSTREAMS; i++) {
        ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                              ABT_TRUE, &pools[i]);
    }

    /* Create work-stealing schedulers */
    for (int i = 0; i < NUM_XSTREAMS; i++) {
        ABT_pool *sched_pools = malloc(sizeof(ABT_pool) * NUM_XSTREAMS);
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

    /* Compute fibonacci */
    ABT_thread main_thread;
    ABT_thread_create(pools[0], fibonacci_ult, &fib_task,
                      ABT_THREAD_ATTR_NULL, &main_thread);
    ABT_thread_free(&main_thread);

    printf("fib(%d) = %d\n", FIB_N, fib_task.result);

    /* Cleanup */
    for (int i = 1; i < NUM_XSTREAMS; i++) {
        ABT_xstream_join(xstreams[i]);
        ABT_xstream_free(&xstreams[i]);
    }

    printf("\nWork-stealing enabled dynamic load balancing across execution streams\n");

    ABT_finalize();
    return 0;
}
