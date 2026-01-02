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

    printf("[Consumer] Polling for key '%s'...\n", args->key);

    char buffer[256];
    size_t vsize;
    int poll_count = 0;

    /* Keep polling until key exists */
    while(1) {
        poll_count++;
        vsize = sizeof(buffer);

        /* Try to get the key (no WAIT mode) */
        yk_return_t ret = yk_get(args->db, YOKAN_MODE_DEFAULT,
                                   args->key, args->ksize,
                                   buffer, &vsize);

        if(ret == YOKAN_SUCCESS) {
            /* Success! Key found */
            break;
        } else {
            /* Key doesn't exist yet, sleep and retry */
            usleep(50000); /* 50ms */
        }
    }

    printf("[Consumer] Received after %d poll attempts: %.*s\n",
           poll_count, (int)vsize, buffer);
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

    /* Get default execution stream and pool */
    ABT_xstream_self(&xstream);
    ABT_xstream_get_main_pools(xstream, 1, &pool);

    const char* key = "polling_key";
    const char* value = "polling_value";

    /* Launch consumer thread that polls for key */
    struct consumer_args args = { db, key, strlen(key) };
    ABT_thread_create(pool, consumer_thread, &args,
                      ABT_THREAD_ATTR_NULL, &consumer);

    /* Give consumer time to start polling */
    usleep(100000); /* 100ms */

    /* Producer: put value (no notify needed) */
    printf("[Producer] Putting value...\n");

    ret = yk_put(db, YOKAN_MODE_DEFAULT,
                 key, strlen(key),
                 value, strlen(value));

    if(ret == YOKAN_SUCCESS) {
        printf("[Producer] Value sent (consumer will find it eventually)\n");
    }

    /* Wait for consumer thread to finish */
    ABT_thread_free(&consumer);

    printf("\n=== Polling approach completed ===\n");
    printf("Note: Multiple network round-trips wasted on polling\n");
    printf("Compare this to wait/notify for better efficiency\n");

    /* Cleanup */
    yk_erase(db, YOKAN_MODE_DEFAULT, key, strlen(key));
    yk_database_handle_release(db);

cleanup:
    if(server_addr != HG_ADDR_NULL) margo_addr_free(mid, server_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
