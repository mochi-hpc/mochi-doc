/*
 * Eventual example: Single-value synchronization
 * Simpler than futures for single-write scenarios
 */

#include <stdio.h>
#include <abt.h>

#define NUM_WAITERS 4

typedef struct {
    int waiter_id;
    ABT_eventual eventual;
} waiter_arg_t;

void writer_thread(void *arg)
{
    ABT_eventual eventual = (ABT_eventual)arg;
    int result = 42;

    printf("Writer: computing result...\n");
    /* Simulate computation */
    for (int i = 0; i < 1000000; i++);

    printf("Writer: setting eventual with value %d\n", result);
    ABT_eventual_set(eventual, &result, sizeof(int));
}

void waiter_thread(void *arg)
{
    waiter_arg_t *waiter = (waiter_arg_t *)arg;

    printf("  Waiter %d: waiting for result...\n", waiter->waiter_id);

    int *result;
    ABT_eventual_wait(waiter->eventual, (void **)&result);

    printf("  Waiter %d: got result = %d\n", waiter->waiter_id, *result);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread writer;
    ABT_thread waiters[NUM_WAITERS];
    ABT_eventual eventual;
    waiter_arg_t waiter_args[NUM_WAITERS];

    ABT_init(argc, argv);

    printf("=== Eventual Example ===\n");
    printf("One writer, multiple waiters\n\n");

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create eventual (no value size parameter needed) */
    ABT_eventual_create(sizeof(int), &eventual);

    /* Create waiters first */
    for (int i = 0; i < NUM_WAITERS; i++) {
        waiter_args[i].waiter_id = i;
        waiter_args[i].eventual = eventual;
        ABT_thread_create(pool, waiter_thread, &waiter_args[i],
                          ABT_THREAD_ATTR_NULL, &waiters[i]);
    }

    /* Create writer */
    ABT_thread_create(pool, writer_thread, eventual,
                      ABT_THREAD_ATTR_NULL, &writer);

    /* Wait for all */
    ABT_thread_free(&writer);
    for (int i = 0; i < NUM_WAITERS; i++) {
        ABT_thread_free(&waiters[i]);
    }

    ABT_eventual_free(&eventual);

    printf("\nEventual: single writer, multiple readers pattern\n");

    ABT_finalize();
    return 0;
}
