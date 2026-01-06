/*
 * Performance measurement with Argobots timing functions
 */

#include <stdio.h>
#include <abt.h>

#define NUM_ITERATIONS 1000
#define NUM_THREADS 4

void computation_work(void *arg)
{
    int id = *(int *)arg;
    volatile long sum = 0;

    for (int i = 0; i < 100000; i++) {
        sum += i;
    }
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_THREADS];
    ABT_timer timer;
    int thread_ids[NUM_THREADS];
    double start_time, end_time, elapsed;

    ABT_init(argc, argv);

    printf("=== Performance Measurement ===\n\n");

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Method 1: Using ABT_get_wtime() */
    printf("Method 1: ABT_get_wtime()\n");
    start_time = ABT_get_wtime();

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < NUM_THREADS; i++) {
            thread_ids[i] = i;
            ABT_thread_create(pool, computation_work, &thread_ids[i],
                              ABT_THREAD_ATTR_NULL, &threads[i]);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            ABT_thread_free(&threads[i]);
        }
    }

    end_time = ABT_get_wtime();
    elapsed = end_time - start_time;
    printf("  Total time: %.6f seconds\n", elapsed);
    printf("  Per iteration: %.6f ms\n", (elapsed / NUM_ITERATIONS) * 1000);
    printf("  Per thread: %.6f us\n\n", (elapsed / (NUM_ITERATIONS * NUM_THREADS)) * 1000000);

    /* Method 2: Using ABT_timer */
    printf("Method 2: ABT_timer\n");
    ABT_timer_create(&timer);

    ABT_timer_start(timer);

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < NUM_THREADS; i++) {
            thread_ids[i] = i;
            ABT_thread_create(pool, computation_work, &thread_ids[i],
                              ABT_THREAD_ATTR_NULL, &threads[i]);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            ABT_thread_free(&threads[i]);
        }
    }

    ABT_timer_stop(timer);
    ABT_timer_read(timer, &elapsed);

    printf("  Total time: %.6f seconds\n", elapsed);
    printf("  Per iteration: %.6f ms\n", (elapsed / NUM_ITERATIONS) * 1000);
    printf("  Per thread: %.6f us\n\n", (elapsed / (NUM_ITERATIONS * NUM_THREADS)) * 1000000);

    ABT_timer_free(&timer);

    printf("Use timing to identify performance bottlenecks\n");

    ABT_finalize();
    return 0;
}
