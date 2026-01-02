/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <yokan/database.h>
#include <yokan/client.h>
#include <margo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct migration_context {
    yk_database_handle_t dest_db;
    size_t* count;
};

static yk_return_t migrate_callback(void* uargs,
                                     size_t i,
                                     const void* key, size_t ksize,
                                     const void* val, size_t vsize)
{
    struct migration_context* ctx = (struct migration_context*)uargs;

    yk_return_t ret = yk_put(ctx->dest_db, YOKAN_MODE_DEFAULT,
                               key, ksize, val, vsize);
    if(ret != YOKAN_SUCCESS) {
        return ret;
    }

    (*ctx->count)++;

    /* Progress reporting */
    if(*ctx->count % 1000 == 0) {
        printf("Migrated %zu items...\n", *ctx->count);
    }

    return YOKAN_SUCCESS;
}

int main(void) {
    /* Initialize Margo */
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "Failed to initialize Margo\n");
        return 1;
    }

    /* Source and destination are both Bedrock-managed providers */
    const char* source_addr_str = "na+sm://127.0.0.1:1234";
    const char* dest_addr_str = "na+sm://127.0.0.1:1235";

    printf("Migrating between Bedrock-managed providers...\n");
    printf("Source: %s\n", source_addr_str);
    printf("Destination: %s\n", dest_addr_str);

    yk_return_t ret;
    yk_client_t client;
    yk_database_handle_t source_db, dest_db;
    hg_addr_t source_addr = HG_ADDR_NULL;
    hg_addr_t dest_addr = HG_ADDR_NULL;

    /* Initialize Yokan client */
    ret = yk_client_init(mid, &client);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to initialize Yokan client\n");
        margo_finalize(mid);
        return 1;
    }

    /* Look up addresses */
    hg_return_t hret = margo_addr_lookup(mid, source_addr_str, &source_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup source address\n");
        goto cleanup;
    }

    hret = margo_addr_lookup(mid, dest_addr_str, &dest_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup destination address\n");
        goto cleanup;
    }

    /* Create database handles */
    ret = yk_database_handle_create(client, source_addr, 42, 1, &source_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create source database handle\n");
        goto cleanup;
    }

    ret = yk_database_handle_create(client, dest_addr, 42, 1, &dest_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create destination database handle\n");
        yk_database_handle_release(source_db);
        goto cleanup;
    }

    /* Populate source */
    printf("\nPopulating source database...\n");
    for(int i = 0; i < 5000; i++) {
        char key[64], value[64];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);
        yk_put(source_db, YOKAN_MODE_DEFAULT, key, strlen(key), value, strlen(value));
    }

    /* Migrate with larger batch size for Bedrock-managed providers */
    size_t migrated = 0;
    struct migration_context ctx = { dest_db, &migrated };

    ret = yk_iter(source_db, YOKAN_MODE_DEFAULT,
                  NULL, 0, NULL, 0,
                  5000,  /* Large batch size for efficiency */
                  migrate_callback,
                  &ctx,
                  NULL);

    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Migration failed\n");
    } else {
        printf("\nMigration completed successfully!\n");
    }

    /* Verify both providers are still operational */
    size_t source_count, dest_count;
    yk_count(source_db, YOKAN_MODE_DEFAULT, &source_count);
    yk_count(dest_db, YOKAN_MODE_DEFAULT, &dest_count);

    printf("Source count: %zu\n", source_count);
    printf("Destination count: %zu\n", dest_count);

    if(dest_count == source_count && dest_count == 5000) {
        printf("SUCCESS: All data migrated between Bedrock providers\n");
    }

    /* Cleanup */
    yk_database_handle_release(dest_db);
    yk_database_handle_release(source_db);

cleanup:
    if(source_addr != HG_ADDR_NULL) margo_addr_free(mid, source_addr);
    if(dest_addr != HG_ADDR_NULL) margo_addr_free(mid, dest_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
