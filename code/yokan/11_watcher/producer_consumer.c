/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <yokan/database.h>
#include <yokan/client.h>
#include <margo.h>
#include <abt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 5

struct shared_state {
    yk_database_handle_t db;
    ABT_mutex mutex;
    int items_produced;
    int items_consumed;
};

struct consumer_args {
    struct shared_state* state;
    int id;
};

struct producer_args {
    struct shared_state* state;
    int id;
};

static void consumer_thread(void* arg)
{
    struct consumer_args* args = (struct consumer_args*)arg;
    struct shared_state* state = args->state;
    int id = args->id;

    while(1) {
        /* Get next work item ID (atomically) */
        ABT_mutex_lock(state->mutex);
        int item_id = state->items_consumed++;
        int consumed = state->items_consumed;
        ABT_mutex_unlock(state->mutex);

        /* Generate unique work item key */
        char key[64];
        snprintf(key, sizeof(key), "work_item_%d", item_id);

        char buffer[256];
        size_t vsize = sizeof(buffer);

        /* Wait for work item and consume it atomically */
        yk_return_t ret = yk_get(state->db, YOKAN_MODE_WAIT | YOKAN_MODE_CONSUME,
                                   key, strlen(key),
                                   buffer, &vsize);

        if(ret == YOKAN_SUCCESS) {
            printf("[Consumer %d] Processing: %.*s\n", id, (int)vsize, buffer);

            /* Simulate work */
            usleep(50000); /* 50ms */
        }

        /* Stop when we've consumed all expected items */
        if(consumed >= NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
            break;
        }
    }

    printf("[Consumer %d] Finished\n", id);
}

static void producer_thread(void* arg)
{
    struct producer_args* args = (struct producer_args*)arg;
    struct shared_state* state = args->state;
    int id = args->id;

    for(int j = 0; j < ITEMS_PER_PRODUCER; j++) {
        /* Get next item ID (atomically) */
        ABT_mutex_lock(state->mutex);
        int item_id = state->items_produced++;
        ABT_mutex_unlock(state->mutex);

        char key[64], work[256];
        snprintf(key, sizeof(key), "work_item_%d", item_id);
        snprintf(work, sizeof(work), "Task %d from producer %d", item_id, id);

        printf("[Producer %d] Creating: %s\n", id, work);

        /* Put work item with notification */
        yk_put(state->db, YOKAN_MODE_NOTIFY,
               key, strlen(key),
               work, strlen(work));

        /* Simulate production delay */
        usleep(100000); /* 100ms */
    }

    printf("[Producer %d] Finished\n", id);
}

int main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server_addr> <provider_id>\n", argv[0]);
        return 1;
    }

    const char* server_addr_str = argv[1];
    uint16_t provider_id = (uint16_t)atoi(argv[2]);

    /* Initialize Margo */
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "Failed to initialize Margo\n");
        return 1;
    }

    yk_return_t ret;
    yk_client_t client;
    yk_database_handle_t db;
    hg_addr_t server_addr = HG_ADDR_NULL;
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_mutex mutex;

    /* Initialize shared state */
    struct shared_state state = { .items_produced = 0, .items_consumed = 0 };

    /* Initialize Yokan client */
    ret = yk_client_init(mid, &client);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to initialize Yokan client\n");
        margo_finalize(mid);
        return 1;
    }

    /* Look up address */
    hg_return_t hret = margo_addr_lookup(mid, server_addr_str, &server_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup server address\n");
        goto cleanup;
    }

    /* Create database handle */
    ret = yk_database_handle_create(client, server_addr, provider_id, 1, &db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create database handle\n");
        goto cleanup;
    }

    state.db = db;

    /* Create mutex for atomic counters */
    ABT_mutex_create(&mutex);
    state.mutex = mutex;

    /* Get default execution stream and pool */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    printf("Starting producer/consumer workflow...\n");

    /* Create consumer threads */
    ABT_thread consumers[NUM_CONSUMERS];
    struct consumer_args consumer_args[NUM_CONSUMERS];

    for(int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_args[i].state = &state;
        consumer_args[i].id = i;
        ABT_thread_create(pool, consumer_thread, &consumer_args[i],
                          ABT_THREAD_ATTR_NULL, &consumers[i]);
    }

    /* Give consumers time to start waiting */
    usleep(100000); /* 100ms */

    /* Create producer threads */
    ABT_thread producers[NUM_PRODUCERS];
    struct producer_args producer_args[NUM_PRODUCERS];

    for(int i = 0; i < NUM_PRODUCERS; i++) {
        producer_args[i].state = &state;
        producer_args[i].id = i;
        ABT_thread_create(pool, producer_thread, &producer_args[i],
                          ABT_THREAD_ATTR_NULL, &producers[i]);
    }

    /* Wait for all threads to complete */
    for(int i = 0; i < NUM_PRODUCERS; i++) {
        ABT_thread_free(&producers[i]);
    }
    for(int i = 0; i < NUM_CONSUMERS; i++) {
        ABT_thread_free(&consumers[i]);
    }

    printf("\n=== Producer/Consumer workflow completed ===\n");
    printf("Items produced: %d\n", state.items_produced);
    printf("Items consumed: %d\n", state.items_consumed);

    /* Cleanup */
    ABT_mutex_free(&mutex);
    yk_database_handle_release(db);

cleanup:
    if(server_addr != HG_ADDR_NULL) margo_addr_free(mid, server_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
