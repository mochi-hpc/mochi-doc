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

    /* Copy key-value pair to destination */
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

    printf("Connected to source and destination databases\n");

    /* Populate source with test data */
    printf("Populating source database...\n");
    for(int i = 0; i < 100; i++) {
        char key[64], value[64];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);

        ret = yk_put(source_db, YOKAN_MODE_DEFAULT,
                     key, strlen(key), value, strlen(value));
        if(ret != YOKAN_SUCCESS) {
            fprintf(stderr, "Failed to put key %s\n", key);
        }
    }
    printf("Source database populated with 100 key/value pairs\n");

    /* Migrate data by iterating over source and copying to destination */
    printf("\nMigrating database...\n");

    size_t migrated_count = 0;
    struct migration_context ctx = { dest_db, &migrated_count };

    ret = yk_iter(source_db, YOKAN_MODE_DEFAULT,
                  NULL, 0,          /* Start from beginning */
                  NULL, 0,          /* No filter */
                  100,              /* Batch size */
                  migrate_callback,
                  &ctx,
                  NULL);            /* No custom options */

    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Migration failed\n");
    } else {
        printf("Migration completed! Migrated %zu key/value pairs\n", migrated_count);
    }

    /* Verify migration */
    size_t source_count, dest_count;
    ret = yk_count(source_db, YOKAN_MODE_DEFAULT, &source_count);
    if(ret == YOKAN_SUCCESS) {
        ret = yk_count(dest_db, YOKAN_MODE_DEFAULT, &dest_count);
    }

    if(ret == YOKAN_SUCCESS) {
        printf("\nVerification:\n");
        printf("  Source database: %zu items\n", source_count);
        printf("  Destination database: %zu items\n", dest_count);

        if(dest_count == source_count && dest_count == 100) {
            printf("SUCCESS: All data migrated!\n");
        } else {
            printf("WARNING: Migration may be incomplete\n");
        }
    }

    /* Spot check a few keys */
    printf("\nSpot check of migrated data:\n");
    for(int i = 0; i < 5; i++) {
        char key[64];
        char value_buf[128];
        size_t vsize = sizeof(value_buf);

        snprintf(key, sizeof(key), "key_%d", i * 20);
        ret = yk_get(dest_db, YOKAN_MODE_DEFAULT,
                     key, strlen(key), value_buf, &vsize);

        if(ret == YOKAN_SUCCESS) {
            printf("  %s -> %.*s\n", key, (int)vsize, value_buf);
        }
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
