/*
 * Progress polling pattern: Non-blocking checks with yield
 * Common in Margo/Mochi for network progress
 */

#include <stdio.h>
#include <abt.h>

#define NUM_REQUESTS 5

typedef struct {
    int request_id;
    int completed;
} async_request_t;

typedef struct {
    async_request_t *requests;
    int num_requests;
} poller_arg_t;

void request_simulator(void *arg)
{
    async_request_t *requests = (async_request_t *)arg;

    /* Simulate async completion of requests */
    for (int i = 0; i < NUM_REQUESTS; i++) {
        /* Simulate delay */
        for (int j = 0; j < 100000 * (i + 1); j++);

        requests[i].completed = 1;
        printf("  Background: Request %d completed\n", requests[i].request_id);
        ABT_self_yield();
    }
}

void progress_poller(void *arg)
{
    poller_arg_t *poller = (poller_arg_t *)arg;
    int completed_count = 0;

    printf("Poller: Starting to poll for completions\n\n");

    while (completed_count < poller->num_requests) {
        /* Poll for completed requests (non-blocking) */
        for (int i = 0; i < poller->num_requests; i++) {
            if (poller->requests[i].completed &&
                poller->requests[i].request_id >= 0) {
                printf("Poller: Detected completion of request %d\n",
                       poller->requests[i].request_id);
                poller->requests[i].request_id = -1;  /* Mark as processed */
                completed_count++;
            }
        }

        /* Yield to let other work progress */
        ABT_self_yield();
    }

    printf("\nPoller: All requests completed\n");
}

int main(int argc, char **argv)
{
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread simulator;
    ABT_thread poller;
    async_request_t requests[NUM_REQUESTS];
    poller_arg_t poller_arg;

    ABT_init(argc, argv);

    printf("=== Progress Polling Pattern ===\n");
    printf("Simulates async I/O or network progress polling\n\n");

    /* Initialize requests */
    for (int i = 0; i < NUM_REQUESTS; i++) {
        requests[i].request_id = i;
        requests[i].completed = 0;
    }

    poller_arg.requests = requests;
    poller_arg.num_requests = NUM_REQUESTS;

    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    /* Create poller thread */
    ABT_thread_create(pool, progress_poller, &poller_arg,
                      ABT_THREAD_ATTR_NULL, &poller);

    /* Create simulator thread */
    ABT_thread_create(pool, request_simulator, requests,
                      ABT_THREAD_ATTR_NULL, &simulator);

    /* Wait for both */
    ABT_thread_free(&poller);
    ABT_thread_free(&simulator);

    printf("\nProgress polling with yield: common pattern in Margo\n");

    ABT_finalize();
    return 0;
}
