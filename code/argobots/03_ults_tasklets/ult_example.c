/*
 * ULT Example: User-Level Threads with stacks
 * ULTs can yield, be migrated, and make recursive calls
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_TASKS 8

/* Recursive fibonacci function - requires stack */
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

typedef struct {
    int task_id;
    int n;
} task_arg_t;

void ult_func(void *arg)
{
    task_arg_t *task = (task_arg_t *)arg;
    int result = fibonacci(task->n);

    int xstream_rank;
    ABT_xstream_self_rank(&xstream_rank);

    printf("ULT %d on ES %d: fib(%d) = %d\n",
           task->task_id, xstream_rank, task->n, result);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_TASKS];
    ABT_thread_attr attr;
    task_arg_t task_args[NUM_TASKS];

    ABT_init(argc, argv);

    printf("=== ULT Example ===\n");
    printf("ULTs have their own stack and can make recursive calls\n\n");

    /* Get primary execution stream and pool */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create thread attributes with custom stack size */
    ABT_thread_attr_create(&attr);
    ABT_thread_attr_set_stacksize(attr, 16384); /* 16KB stack */

    /* Create ULTs */
    for (int i = 0; i < NUM_TASKS; i++) {
        task_args[i].task_id = i;
        task_args[i].n = 10 + i;  /* fib(10) through fib(17) */

        ABT_thread_create(pool, ult_func, &task_args[i], attr, &threads[i]);
    }

    /* Free attribute */
    ABT_thread_attr_free(&attr);

    /* Wait for all ULTs */
    for (int i = 0; i < NUM_TASKS; i++) {
        ABT_thread_free(&threads[i]);
    }

    printf("\nAll ULTs completed\n");
    printf("Note: ULTs can yield, be suspended/resumed, and migrated\n");

    ABT_finalize();
    return 0;
}
