/*
 * Tasklet Example: Stackless work units
 * Tasklets are lighter weight but cannot yield or make deep recursive calls
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_TASKS 8

typedef struct {
    int task_id;
    int value;
} task_arg_t;

/* Simple computation that doesn't require much stack */
void tasklet_func(void *arg)
{
    task_arg_t *task = (task_arg_t *)arg;
    int result = task->value * task->value;

    int xstream_rank;
    ABT_xstream_self_rank(&xstream_rank);

    printf("Tasklet %d on ES %d: %d^2 = %d\n",
           task->task_id, xstream_rank, task->value, result);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_task tasks[NUM_TASKS];
    task_arg_t task_args[NUM_TASKS];

    ABT_init(argc, argv);

    printf("=== Tasklet Example ===\n");
    printf("Tasklets are stackless and execute to completion\n\n");

    /* Get primary execution stream and pool */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create tasklets */
    for (int i = 0; i < NUM_TASKS; i++) {
        task_args[i].task_id = i;
        task_args[i].value = 10 + i;

        ABT_task_create(pool, tasklet_func, &task_args[i], &tasks[i]);
    }

    /* Wait for all tasklets */
    for (int i = 0; i < NUM_TASKS; i++) {
        ABT_task_free(&tasks[i]);
    }

    printf("\nAll tasklets completed\n");
    printf("Note: Tasklets have lower overhead but cannot yield or migrate\n");

    ABT_finalize();
    return 0;
}
