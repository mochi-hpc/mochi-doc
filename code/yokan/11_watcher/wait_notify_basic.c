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
};

static void consumer_thread(void* arg)
{
    struct consumer_args* args = (struct consumer_args*)arg;

    printf("[Consumer] Waiting for key '%s'...\n", args->key);

    char buffer[256];
    size_t vsize = sizeof(buffer);

    /* Use YOKAN_MODE_WAIT to block until key appears */
    yk_return_t ret = yk_get(args->db, YOKAN_MODE_WAIT,
                               args->key, args->ksize,
                               buffer, &vsize);

    if(ret == YOKAN_SUCCESS) {
        printf("[Consumer] Received: %.*s\n", (int)vsize, buffer);
    } else {
        fprintf(stderr, "[Consumer] Error getting key\n");
    }
}

int main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server_addr> <provider_id>\n", argv[0]);
        return 1;
    }

    const char* server_addr_str = argv[1];
    uint16_t provider_id = (uint16_t)atoi(argv[2]);

    /* Initialize Margo and Argobots */
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
    ABT_thread consumer;

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

    /* Get default execution stream and pool for threading */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    const char* key = "notification_key";
    const char* value = "notification_value";

    /* Launch consumer thread that waits for key */
    struct consumer_args args = { db, key, strlen(key) };
    ABT_thread_create(pool, consumer_thread, &args,
                      ABT_THREAD_ATTR_NULL, &consumer);

    /* Give consumer time to start waiting */
    usleep(100000); /* 100ms */

    /* Producer: put value with notification */
    printf("[Producer] Putting value with notification...\n");

    /* Use YOKAN_MODE_NOTIFY to wake up waiting consumers */
    ret = yk_put(db, YOKAN_MODE_NOTIFY,
                 key, strlen(key),
                 value, strlen(value));

    if(ret == YOKAN_SUCCESS) {
        printf("[Producer] Value sent\n");
    }

    /* Wait for consumer thread to finish */
    ABT_thread_free(&consumer);

    printf("\n=== Wait/Notify completed successfully ===\n");

    /* Cleanup */
    yk_database_handle_release(db);

cleanup:
    if(server_addr != HG_ADDR_NULL) margo_addr_free(mid, server_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
