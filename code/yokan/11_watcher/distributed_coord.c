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

#define NUM_RANKS 4

/* Simulate distributed barrier synchronization */
static void barrier_sync(yk_database_handle_t db, int rank, int num_ranks)
{
    char my_key[64];
    const char* ready_value = "ready";

    snprintf(my_key, sizeof(my_key), "barrier_%d", rank);

    printf("[Rank %d] Announcing readiness\n", rank);

    /* Announce that this rank is ready */
    yk_put(db, YOKAN_MODE_NOTIFY,
           my_key, strlen(my_key),
           ready_value, strlen(ready_value));

    /* Wait for all other ranks to be ready */
    for(int i = 0; i < num_ranks; i++) {
        if(i == rank) continue;

        char other_key[64];
        char buffer[256];
        size_t vsize = sizeof(buffer);

        snprintf(other_key, sizeof(other_key), "barrier_%d", i);

        printf("[Rank %d] Waiting for rank %d\n", rank, i);

        yk_get(db, YOKAN_MODE_WAIT,
               other_key, strlen(other_key),
               buffer, &vsize);
    }

    printf("[Rank %d] All ranks synchronized!\n", rank);
}

struct rank_args {
    yk_database_handle_t db;
    int rank;
    int num_ranks;
};

static void rank_thread(void* arg)
{
    struct rank_args* args = (struct rank_args*)arg;

    /* Simulate different arrival times */
    usleep(args->rank * 100000); /* rank * 100ms */

    /* Execute barrier synchronization */
    barrier_sync(args->db, args->rank, args->num_ranks);

    /* Continue with synchronized work */
    printf("[Rank %d] Proceeding with synchronized work\n", args->rank);
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

    printf("Simulating distributed barrier with %d ranks\n", NUM_RANKS);

    /* Simulate multiple distributed processes */
    ABT_thread ranks[NUM_RANKS];
    struct rank_args args[NUM_RANKS];

    for(int rank = 0; rank < NUM_RANKS; rank++) {
        args[rank].db = db;
        args[rank].rank = rank;
        args[rank].num_ranks = NUM_RANKS;

        ABT_thread_create(pool, rank_thread, &args[rank],
                          ABT_THREAD_ATTR_NULL, &ranks[rank]);
    }

    /* Wait for all ranks to complete */
    for(int rank = 0; rank < NUM_RANKS; rank++) {
        ABT_thread_free(&ranks[rank]);
    }

    printf("\n=== Distributed coordination completed ===\n");

    /* Cleanup barrier keys */
    for(int i = 0; i < NUM_RANKS; i++) {
        char key[64];
        snprintf(key, sizeof(key), "barrier_%d", i);
        yk_erase(db, YOKAN_MODE_DEFAULT, key, strlen(key));
    }

    /* Cleanup */
    yk_database_handle_release(db);

cleanup:
    if(server_addr != HG_ADDR_NULL) margo_addr_free(mid, server_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
