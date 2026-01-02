/*
 * Work-unit keys: Thread-local storage for ULTs
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>

#define NUM_THREADS 4

ABT_key thread_id_key;

typedef struct {
    int value;
} thread_local_data_t;

void thread_func(void *arg)
{
    int thread_id = *(int *)arg;

    /* Allocate thread-local data */
    thread_local_data_t *local = malloc(sizeof(thread_local_data_t));
    local->value = thread_id * 100;

    /* Associate with this ULT */
    ABT_key_set(thread_id_key, local);

    printf("Thread %d: set thread-local value = %d\n", thread_id, local->value);

    /* Simulate some work */
    for (int i = 0; i < 3; i++) {
        /* Retrieve thread-local data */
        thread_local_data_t *retrieved;
        ABT_key_get(thread_id_key, (void **)&retrieved);

        printf("  Thread %d iteration %d: local value = %d\n",
               thread_id, i, retrieved->value);

        ABT_thread_yield();
    }

    /* Cleanup: free thread-local data */
    thread_local_data_t *local_final;
    ABT_key_get(thread_id_key, (void **)&local_final);
    free(local_final);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    ABT_init(argc, argv);

    printf("=== Work-Unit Keys (Thread-Local Storage) ===\n\n");

    /* Create key for thread-local data */
    ABT_key_create(NULL, &thread_id_key);

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        ABT_thread_create(pool, thread_func, &thread_ids[i],
                          ABT_THREAD_ATTR_NULL, &threads[i]);
    }

    /* Wait for all */
    for (int i = 0; i < NUM_THREADS; i++) {
        ABT_thread_free(&threads[i]);
    }

    /* Free key */
    ABT_key_free(&thread_id_key);

    printf("\nEach ULT maintained its own thread-local data\n");

    ABT_finalize();
    return 0;
}
