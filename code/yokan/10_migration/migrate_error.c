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

    /* This may fail if destination has issues */
    yk_return_t ret = yk_put(ctx->dest_db, YOKAN_MODE_DEFAULT,
                               key, ksize, val, vsize);
    if(ret != YOKAN_SUCCESS) {
        return ret;
    }

    (*ctx->count)++;
    return YOKAN_SUCCESS;
}

int main(int argc, char** argv) {
    if(argc != 5) {
        fprintf(stderr, "Usage: %s <source_addr> <source_provider_id>"
                       " <dest_addr> <dest_provider_id>\n", argv[0]);
        return 1;
    }

    const char* source_addr_str = argv[1];
    uint16_t source_provider_id = (uint16_t)atoi(argv[2]);
    const char* dest_addr_str = argv[3];
    uint16_t dest_provider_id = (uint16_t)atoi(argv[4]);

    /* Initialize Margo */
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "Failed to initialize Margo\n");
        return 1;
    }

    yk_return_t ret;
    yk_client_t client;
    yk_database_handle_t source_db = YOKAN_DATABASE_HANDLE_NULL;
    yk_database_handle_t dest_db = YOKAN_DATABASE_HANDLE_NULL;
    hg_addr_t source_addr = HG_ADDR_NULL;
    hg_addr_t dest_addr = HG_ADDR_NULL;

    /* Initialize Yokan client */
    ret = yk_client_init(mid, &client);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to initialize Yokan client\n");
        margo_finalize(mid);
        return 1;
    }

    /* Look up source address */
    hg_return_t hret = margo_addr_lookup(mid, source_addr_str, &source_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup source address\n");
        goto cleanup;
    }

    /* Create source database handle */
    ret = yk_database_handle_create(client, source_addr, source_provider_id,
                                     1, &source_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create source database handle\n");
        goto cleanup;
    }

    /* Populate source with test data */
    printf("Populating source database...\n");
    for(int i = 0; i < 100; i++) {
        char key[64], value[64];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);
        yk_put(source_db, YOKAN_MODE_DEFAULT, key, strlen(key), value, strlen(value));
    }
    printf("Source populated with 100 items\n");

    printf("\nAttempting migration...\n");

    /* Try to look up and connect to destination */
    hret = margo_addr_lookup(mid, dest_addr_str, &dest_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "\nMigration failed: Could not lookup destination address\n");
        fprintf(stderr, "  -> Check that the destination address is valid\n");
        fprintf(stderr, "  -> Verify the destination service is running\n");
        goto show_source_intact;
    }

    ret = yk_database_handle_create(client, dest_addr, dest_provider_id,
                                     1, &dest_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "\nMigration failed: Could not connect to destination provider\n");
        fprintf(stderr, "  -> Invalid destination provider ID\n");
        fprintf(stderr, "  -> Verify the destination provider exists\n");
        goto show_source_intact;
    }

    /* Attempt migration */
    size_t migrated_count = 0;
    struct migration_context ctx = { dest_db, &migrated_count };

    ret = yk_iter(source_db, YOKAN_MODE_DEFAULT,
                  NULL, 0, NULL, 0, 100,
                  migrate_callback, &ctx, NULL);

    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "\nMigration failed during iteration\n");
        fprintf(stderr, "  -> Error code: %d\n", ret);
        fprintf(stderr, "  -> Destination may not allow writes\n");
        fprintf(stderr, "  -> Network issues may have occurred\n");
        goto show_source_intact;
    }

    printf("Migration succeeded! Migrated %zu key/value pairs\n", migrated_count);
    goto cleanup;

show_source_intact:
    {
        /* Source database is still intact after failed migration */
        size_t count;
        ret = yk_count(source_db, YOKAN_MODE_DEFAULT, &count);
        if(ret == YOKAN_SUCCESS) {
            printf("\nSource database still contains %zu key/value pairs\n", count);
            printf("Data was not lost - source remains intact\n");
        }
    }

cleanup:
    if(dest_db != YOKAN_DATABASE_HANDLE_NULL)
        yk_database_handle_release(dest_db);
    if(source_db != YOKAN_DATABASE_HANDLE_NULL)
        yk_database_handle_release(source_db);
    if(source_addr != HG_ADDR_NULL) margo_addr_free(mid, source_addr);
    if(dest_addr != HG_ADDR_NULL) margo_addr_free(mid, dest_addr);
    yk_client_finalize(client);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS) ? 0 : 1;
}
