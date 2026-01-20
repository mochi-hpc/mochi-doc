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

struct consumer_args {
    yk_database_handle_t db;
    const char* key;
    size_t ksize;
    int id;
};

static void consumer_thread(void* arg)
{
    struct consumer_args* args = (struct consumer_args*)arg;

    printf("[Consumer %d] Waiting for event...\n", args->id);

    char buffer[256];
    size_t vsize = sizeof(buffer);

    /* All consumers wait for the same key */
    yk_return_t ret = yk_get(args->db, YOKAN_MODE_WAIT,
                               args->key, args->ksize,
                               buffer, &vsize);

    if(ret == YOKAN_SUCCESS) {
        printf("[Consumer %d] Received: %.*s\n", args->id, (int)vsize, buffer);
    } else {
        fprintf(stderr, "[Consumer %d] Error getting key\n", args->id);
    }
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

    /* Get default execution stream and pool */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    const char* key = "broadcast_event";
    const char* value = "event_data";

    printf("Starting multiple waiters...\n");

    /* Create multiple consumer threads */
#define NUM_CONSUMERS 5
    ABT_thread consumers[NUM_CONSUMERS];
    struct consumer_args args[NUM_CONSUMERS];

    for(int i = 0; i < NUM_CONSUMERS; i++) {
        args[i].db = db;
        args[i].key = key;
        args[i].ksize = strlen(key);
        args[i].id = i;

        ABT_thread_create(pool, consumer_thread, &args[i],
                          ABT_THREAD_ATTR_NULL, &consumers[i]);
    }

    /* Give consumers time to start waiting */
    usleep(200000); /* 200ms */

    /* Single notification wakes all waiters */
    printf("\n[Producer] Broadcasting event to all waiters...\n");

    ret = yk_put(db, YOKAN_MODE_NOTIFY,
                 key, strlen(key),
                 value, strlen(value));

    if(ret == YOKAN_SUCCESS) {
        printf("[Producer] Broadcast sent\n");
    }

    /* Wait for all consumers to complete */
    for(int i = 0; i < NUM_CONSUMERS; i++) {
        ABT_thread_free(&consumers[i]);
    }

    printf("\n=== All consumers received the broadcast ===\n");

    /* Cleanup */
    yk_database_handle_release(db);

cleanup:
    if(server_addr != HG_ADDR_NULL) margo_addr_free(mid, server_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
