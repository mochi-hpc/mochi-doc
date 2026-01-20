/*
 * Simple ULT Example: Lightweight work units
 * All work units are ULTs (User-Level Threads)
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_TASKS 8

typedef struct {
    int task_id;
    int value;
} task_arg_t;

/* Simple computation */
void simple_func(void *arg)
{
    task_arg_t *task = (task_arg_t *)arg;
    int result = task->value * task->value;

    int xstream_rank;
    ABT_xstream_self_rank(&xstream_rank);

    printf("ULT %d on ES %d: %d^2 = %d\n",
           task->task_id, xstream_rank, task->value, result);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_TASKS];
    task_arg_t task_args[NUM_TASKS];

    ABT_init(argc, argv);

    printf("=== Simple ULT Example ===\n");
    printf("ULTs can be used for all types of work\n\n");

    /* Get primary execution stream and pool */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create ULTs with default attributes */
    for (int i = 0; i < NUM_TASKS; i++) {
        task_args[i].task_id = i;
        task_args[i].value = 10 + i;

        ABT_thread_create(pool, simple_func, &task_args[i],
                         ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all ULTs */
    for (int i = 0; i < NUM_TASKS; i++) {
        ABT_thread_free(&threads[i]);
    }

    printf("\nAll ULTs completed\n");

    ABT_finalize();
    return 0;
}
