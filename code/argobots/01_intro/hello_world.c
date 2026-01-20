/*
 * Basic Argobots example: Creating and executing a simple ULT
 */

#include <stdio.h>
#include <abt.h>

/* Thread function that will be executed by the ULT */
void hello_world_fn(void *arg)
{
    int thread_id = *((int *)arg);
    printf("Hello from ULT %d!\n", thread_id);
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;  /* Execution stream handle */
    ABT_pool pool;        /* Pool handle */
    ABT_thread thread;    /* ULT handle */
    int thread_arg = 1;

    /* Step 1: Initialize Argobots */
    ABT_init(argc, argv);
    printf("Argobots initialized\n");

    /* Step 2: Get the primary execution stream (created automatically by ABT_init) */
    ABT_xstream_self(&xstream);
    printf("Got primary execution stream\n");

    /* Step 3: Get the pool associated with the primary execution stream */
    ABT_xstream_get_main_pools(xstream, 1, &pool);
    printf("Got main pool\n");

    /* Step 4: Create a ULT and add it to the pool */
    ABT_thread_create(pool, hello_world_fn, &thread_arg,
                      ABT_THREAD_ATTR_NULL, &thread);
    printf("Created ULT\n");

    /* Step 5: Wait for the ULT to complete */
    ABT_thread_join(thread);
    printf("ULT completed\n");

    /* Step 6: Free the ULT */
    ABT_thread_free(&thread);
    printf("ULT freed\n");

    /* Step 7: Finalize Argobots */
    ABT_finalize();
    printf("Argobots finalized\n");

    return 0;
}
