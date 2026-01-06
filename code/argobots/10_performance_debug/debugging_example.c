/*
 * Debugging with Argobots info functions
 */

#include <stdio.h>
#include <abt.h>

#define NUM_XSTREAMS 2
#define NUM_THREADS 3

void worker_thread(void *arg)
{
    int id = *(int *)arg;
    printf("Worker %d executing\n", id);
}

int main(int argc, char **argv)
{
    ABT_xstream xstreams[NUM_XSTREAMS];
    ABT_thread threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    ABT_init(argc, argv);

    printf("=== Debugging with Info Functions ===\n\n");

    /* Print Argobots configuration */
    printf("1. Argobots Configuration:\n");
    ABT_info_print_config(stdout);
    printf("\n");

    /* Create additional xstream */
    ABT_xstream_self(&xstreams[0]);
    ABT_xstream_create(ABT_SCHED_NULL, &xstreams[1]);

    /* Create threads on primary xstream */
    ABT_pool pool;
    ABT_xstream_get_main_pools(xstreams[0], 1, &pool);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        ABT_thread_create(pool, worker_thread, &thread_ids[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Print all execution streams */
    printf("2. All Execution Streams:\n");
    ABT_info_print_all_xstreams(stdout);
    printf("\n");

    /* Print specific thread information */
    printf("3. Thread Information:\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_state state;
        ABT_thread_get_state(threads[i], &state);
        printf("  Thread %d state: %d\n", i, state);
    }
    printf("\n");

    /* Wait for threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    /* Cleanup */
    ABT_xstream_join(xstreams[1]);
    ABT_xstream_free(&xstreams[1]);

    printf("Info functions help debug Argobots applications\n");

    ABT_finalize();
    return 0;
}
