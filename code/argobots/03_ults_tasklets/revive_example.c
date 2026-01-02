/*
 * Work Unit Reuse Example: Reviving ULTs and Tasklets
 * Instead of creating/destroying work units, reuse them for efficiency
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_ITERATIONS 4
#define NUM_UNITS 4

typedef struct {
    int iteration;
    int unit_id;
} work_arg_t;

void work_func(void *arg)
{
    work_arg_t *work = (work_arg_t *)arg;
    printf("  Work unit %d, iteration %d\n", work->unit_id, work->iteration);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_UNITS];
    work_arg_t thread_args[NUM_UNITS];

    ABT_init(argc, argv);

    printf("=== Work Unit Revive Example ===\n");
    printf("Reusing ULTs across multiple iterations\n\n");

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Initial creation of ULTs */
    printf("Iteration 0 (initial creation):\n");
    for (int i = 0; i < NUM_UNITS; i++) {
        thread_args[i].unit_id = i;
        thread_args[i].iteration = 0;
        ABT_thread_create(pool, work_func, &thread_args[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for first iteration */
    for (int i = 0; i < NUM_UNITS; i++) {
        ABT_thread_join(threads[i]);
    }

    /* Revive and reuse ULTs for additional iterations */
    for (int iter = 1; iter < NUM_ITERATIONS; iter++) {
        printf("\nIteration %d (reviving existing ULTs):\n", iter);

        for (int i = 0; i < NUM_UNITS; i++) {
            thread_args[i].iteration = iter;

            /* Revive the ULT instead of creating a new one */
            ABT_thread_revive(pool, work_func, &thread_args[i], &threads[i]);
        }

        /* Wait for this iteration */
        for (int i = 0; i < NUM_UNITS; i++) {
            ABT_thread_join(threads[i]);
        }
    }

    /* Finally free the ULTs */
    for (int i = 0; i < NUM_UNITS; i++) {
        ABT_thread_free(&threads[i]);
    }

    printf("\n=== Tasklet Revive Example ===\n");
    printf("Reusing tasklets across multiple iterations\n\n");

    ABT_task tasks[NUM_UNITS];

    /* Initial creation of tasklets */
    printf("Iteration 0 (initial creation):\n");
    for (int i = 0; i < NUM_UNITS; i++) {
        thread_args[i].iteration = 0;
        ABT_task_create(pool, work_func, &thread_args[i], &tasks[i]);
    }

    /* Wait for first iteration */
    for (int i = 0; i < NUM_UNITS; i++) {
        ABT_task_state state;
        do {
            ABT_task_get_state(tasks[i], &state);
        } while (state != ABT_TASK_STATE_TERMINATED);
    }

    /* Revive and reuse tasklets */
    for (int iter = 1; iter < NUM_ITERATIONS; iter++) {
        printf("\nIteration %d (reviving existing tasklets):\n", iter);

        for (int i = 0; i < NUM_UNITS; i++) {
            thread_args[i].iteration = iter;
            ABT_task_revive(pool, work_func, &thread_args[i], &tasks[i]);
        }

        /* Wait for this iteration */
        for (int i = 0; i < NUM_UNITS; i++) {
            ABT_task_state state;
            do {
                ABT_task_get_state(tasks[i], &state);
            } while (state != ABT_TASK_STATE_TERMINATED);
        }
    }

    /* Free tasklets */
    for (int i = 0; i < NUM_UNITS; i++) {
        ABT_task_free(&tasks[i]);
    }

    printf("\nReviving work units avoids creation/destruction overhead\n");
    printf("This is especially important for high-frequency operations\n");

    ABT_finalize();
    return 0;
}
