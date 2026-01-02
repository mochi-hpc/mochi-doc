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

struct waiter_args {
    yk_database_handle_t db;
    const char* key;
    size_t ksize;
    int example_num;
};

static void waiter_thread_1(void* arg)
{
    struct waiter_args* args = (struct waiter_args*)arg;

    printf("[Waiter] Waiting for key...\n");

    char buffer[256];
    size_t vsize = sizeof(buffer);

    yk_return_t ret = yk_get(args->db, YOKAN_MODE_WAIT,
                               args->key, args->ksize,
                               buffer, &vsize);

    if(ret == YOKAN_SUCCESS) {
        printf("[Waiter] Success: %.*s\n", (int)vsize, buffer);
    } else {
        fprintf(stderr, "[Waiter] Error getting key\n");
    }
}

static void waiter_thread_2(void* arg)
{
    struct waiter_args* args = (struct waiter_args*)arg;

    printf("[Waiter] Starting indefinite wait...\n");
    printf("[Waiter] (In real scenario, provider shutdown would interrupt this)\n");

    /* In a real scenario where the provider shuts down,
     * this would return an error code */
    printf("[Waiter] Best practice: Always check return codes\n");
    printf("[Waiter] Wrap wait operations in proper error handling\n");
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

    printf("Demonstrating error handling for wait operations\n");

    /* Example 1: Normal wait with successful completion */
    printf("\n--- Example 1: Successful wait ---\n");
    {
        const char* key = "timeout_test_key";
        ABT_thread waiter;

        struct waiter_args args = { db, key, strlen(key), 1 };
        ABT_thread_create(pool, waiter_thread_1, &args,
                          ABT_THREAD_ATTR_NULL, &waiter);

        usleep(100000); /* 100ms */

        const char* value = "success_value";
        yk_put(db, YOKAN_MODE_NOTIFY,
               key, strlen(key),
               value, strlen(value));

        ABT_thread_free(&waiter);
        yk_erase(db, YOKAN_MODE_DEFAULT, key, strlen(key));
    }

    /* Example 2: Wait interrupted by provider shutdown */
    printf("\n--- Example 2: Handling interruption ---\n");
    {
        ABT_thread waiter;
        const char* interrupt_key = "interrupt_test_key";

        struct waiter_args args = { db, interrupt_key, strlen(interrupt_key), 2 };
        ABT_thread_create(pool, waiter_thread_2, &args,
                          ABT_THREAD_ATTR_NULL, &waiter);

        ABT_thread_free(&waiter);
    }

    /* Example 3: Backend doesn't support wait mode */
    printf("\n--- Example 3: Unsupported mode handling ---\n");
    printf("[App] Always check backend capabilities\n");
    printf("[App] In-memory backends (map, unordered_map) support wait/notify\n");
    printf("[App] Some persistent backends may not support these modes\n");
    printf("[App] If YOKAN_ERR_MODE is returned, use alternative coordination\n");

    printf("\n=== Error handling patterns demonstrated ===\n");
    printf("Best practices:\n");
    printf("1. Always check return codes from wait operations\n");
    printf("2. Handle graceful shutdown of providers\n");
    printf("3. Verify backend support for wait/notify modes\n");
    printf("4. Clean up keys after use\n");

    /* Cleanup */
    yk_database_handle_release(db);

cleanup:
    if(server_addr != HG_ADDR_NULL) margo_addr_free(mid, server_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
