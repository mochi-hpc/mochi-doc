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

#define MAX_KEYS 1000

struct migration_context {
    yk_database_handle_t dest_db;
    char (*keys)[128];
    size_t* key_sizes;
    size_t* count;
};

static yk_return_t migrate_callback(void* uargs,
                                     size_t i,
                                     const void* key, size_t ksize,
                                     const void* val, size_t vsize)
{
    struct migration_context* ctx = (struct migration_context*)uargs;

    /* Copy to destination */
    yk_return_t ret = yk_put(ctx->dest_db, YOKAN_MODE_DEFAULT,
                               key, ksize, val, vsize);
    if(ret != YOKAN_SUCCESS) {
        return ret;
    }

    /* Remember key for deletion */
    size_t idx = *ctx->count;
    if(idx < MAX_KEYS && ksize < 128) {
        memcpy(ctx->keys[idx], key, ksize);
        ctx->key_sizes[idx] = ksize;
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

    /* Allocate memory for storing keys */
    char (*keys)[128] = malloc(MAX_KEYS * sizeof(*keys));
    size_t* key_sizes = malloc(MAX_KEYS * sizeof(size_t));
    if(!keys || !key_sizes) {
        fprintf(stderr, "Memory allocation failed\n");
        free(keys);
        free(key_sizes);
        margo_finalize(mid);
        return 1;
    }

    /* Initialize Yokan client */
    ret = yk_client_init(mid, &client);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to initialize Yokan client\n");
        goto cleanup_mem;
    }

    /* Look up addresses */
    hg_return_t hret = margo_addr_lookup(mid, source_addr_str, &source_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup source address\n");
        goto cleanup_client;
    }

    hret = margo_addr_lookup(mid, dest_addr_str, &dest_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup destination address\n");
        goto cleanup_client;
    }

    /* Create database handles */
    ret = yk_database_handle_create(client, source_addr, source_provider_id,
                                     1, &source_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create source database handle\n");
        goto cleanup_client;
    }

    ret = yk_database_handle_create(client, dest_addr, dest_provider_id,
                                     1, &dest_db);
    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Failed to create destination database handle\n");
        yk_database_handle_release(source_db);
        goto cleanup_client;
    }

    /* Populate source with test data */
    printf("Populating source database...\n");
    for(int i = 0; i < 100; i++) {
        char key[64], value[64];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);
        yk_put(source_db, YOKAN_MODE_DEFAULT, key, strlen(key), value, strlen(value));
    }
    printf("Source database populated with 100 key/value pairs\n");

    printf("\nMigrating with source removal...\n");

    /* Migrate data and collect keys */
    size_t migrated_count = 0;
    struct migration_context ctx = { dest_db, keys, key_sizes, &migrated_count };

    ret = yk_iter(source_db, YOKAN_MODE_DEFAULT,
                  NULL, 0, NULL, 0, 100,
                  migrate_callback, &ctx, NULL);

    if(ret != YOKAN_SUCCESS) {
        fprintf(stderr, "Migration failed\n");
    } else {
        printf("Data copied to destination. Removing from source...\n");

        /* Remove keys from source */
        for(size_t i = 0; i < migrated_count && i < MAX_KEYS; i++) {
            yk_erase(source_db, YOKAN_MODE_DEFAULT, keys[i], key_sizes[i]);
        }

        printf("Migration with removal completed!\n");
    }

    /* Verify */
    size_t source_count, dest_count;
    yk_count(source_db, YOKAN_MODE_DEFAULT, &source_count);
    yk_count(dest_db, YOKAN_MODE_DEFAULT, &dest_count);

    printf("\nVerification:\n");
    printf("  Source database: %zu key/value pairs\n", source_count);
    printf("  Destination database: %zu key/value pairs\n", dest_count);

    if(source_count == 0 && dest_count == 100) {
        printf("SUCCESS: Source data removed, all data migrated!\n");
    } else {
        printf("WARNING: Unexpected state\n");
    }

    /* Cleanup */
    yk_database_handle_release(dest_db);
    yk_database_handle_release(source_db);

cleanup_client:
    if(source_addr != HG_ADDR_NULL) margo_addr_free(mid, source_addr);
    if(dest_addr != HG_ADDR_NULL) margo_addr_free(mid, dest_addr);
    yk_client_finalize(client);

cleanup_mem:
    free(keys);
    free(key_sizes);
    margo_finalize(mid);

    return (ret == YOKAN_SUCCESS && source_count == 0) ? 0 : 1;
}
