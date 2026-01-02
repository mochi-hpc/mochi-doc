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
    size_t* batch_count;
    size_t* total_count;
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

    (*ctx->batch_count)++;
    (*ctx->total_count)++;
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
    ret = yk_database_handle_create(client, source_addr, source_provider_id,
                                     1, &source_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create source database handle\n");
        goto cleanup;
    }

    ret = yk_database_handle_create(client, dest_addr, dest_provider_id,
                                     1, &dest_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create destination database handle\n");
        yk_database_handle_release(source_db);
        goto cleanup;
    }

    /* Populate source with data having various prefixes */
    printf("Populating source database...\n");
    const char* prefixes[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    size_t num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);

    for(size_t p = 0; p < num_prefixes; p++) {
        for(int i = 0; i < 100; i++) {
            char key[64], value[64];
            snprintf(key, sizeof(key), "%s_key_%d", prefixes[p], i);
            snprintf(value, sizeof(value), "value_%d", i);
            yk_put(source_db, YOKAN_MODE_DEFAULT, key, strlen(key), value, strlen(value));
        }
    }
    printf("Source populated with %zu items\n", num_prefixes * 100);

    printf("\nStarting incremental migration...\n");
    printf("Migrating in %zu batches by prefix\n", num_prefixes);

    size_t total_migrated = 0;

    for(size_t p = 0; p < num_prefixes; p++) {
        printf("\n[%zu/%zu] Migrating keys with prefix '%s'...\n",
               p + 1, num_prefixes, prefixes[p]);

        size_t batch_migrated = 0;
        struct migration_context ctx = { dest_db, &batch_migrated, &total_migrated };

        /* Migrate keys with this specific prefix */
        char filter[64];
        snprintf(filter, sizeof(filter), "%s_", prefixes[p]);

        ret = yk_iter(source_db, YOKAN_MODE_INCLUSIVE,
                      NULL, 0,                    /* Start from beginning */
                      filter, strlen(filter),     /* Filter by prefix */
                      1000,                       /* Batch size */
                      migrate_callback,
                      &ctx,
                      NULL);

        if(ret != YOKAN_SUCCESS) {
            fprintf(stderr, "  WARNING: Failed to migrate prefix '%s'\n", prefixes[p]);
            fprintf(stderr, "  Continuing with next prefix...\n");
        } else {
            printf("  Prefix '%s': %zu items migrated\n", prefixes[p], batch_migrated);

            /* Progress reporting */
            int progress = ((p + 1) * 100) / num_prefixes;
            printf("  Overall progress: %d%% (%zu items total)\n",
                   progress, total_migrated);
        }
    }

    printf("\n=== Incremental migration completed ===\n");

    /* Verify final state */
    size_t source_count, dest_count;
    yk_count(source_db, YOKAN_MODE_DEFAULT, &source_count);
    yk_count(dest_db, YOKAN_MODE_DEFAULT, &dest_count);

    printf("\nVerification:\n");
    printf("  Source database: %zu items\n", source_count);
    printf("  Destination database: %zu items\n", dest_count);
    printf("  Total migrated: %zu items\n", total_migrated);

    if(dest_count == source_count && dest_count == total_migrated) {
        printf("SUCCESS: All data migrated incrementally!\n");
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
