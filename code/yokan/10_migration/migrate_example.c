#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <remi/remi-server.h>
#include <remi/remi-client.h>
#include <yokan/server.h>
#include <yokan/client.h>
#include <yokan/database.h>

int main(int argc, char** argv)
{
    // Initialize Margo
    margo_instance_id mid = margo_init("ofi+tcp", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    // Get our own address
    hg_addr_t addr;
    hg_return_t hret = margo_addr_self(mid, &addr);
    assert(hret == HG_SUCCESS);

    char addr_str[128];
    hg_size_t bufsize = 128;
    hret = margo_addr_to_string(mid, addr_str, &bufsize, addr);
    assert(hret == HG_SUCCESS);

    // Register a REMI provider (needed for destination)
    remi_provider_t remi_provider;
    int ret = remi_provider_register(
            mid, ABT_IO_INSTANCE_NULL,
            3, ABT_POOL_NULL, &remi_provider);
    assert(ret == REMI_SUCCESS);

    // Create a REMI client (needed for source)
    remi_client_t remi_client;
    ret = remi_client_init(mid, ABT_IO_INSTANCE_NULL, &remi_client);
    assert(ret == REMI_SUCCESS);

    // Register source Yokan provider with a map database
    yk_provider_t provider1;
    struct yk_provider_args args1 = YOKAN_PROVIDER_ARGS_INIT;
    args1.remi.client = remi_client;
    args1.remi.provider = REMI_PROVIDER_NULL;
    const char* config1 = "{ \"database\": { \"type\": \"map\" } }";
    yk_return_t yret = yk_provider_register(
            mid, 1, config1, &args1, &provider1);
    assert(yret == YOKAN_SUCCESS);

    // Register destination Yokan provider with EMPTY config
    yk_provider_t provider2;
    struct yk_provider_args args2 = YOKAN_PROVIDER_ARGS_INIT;
    args2.remi.client = REMI_CLIENT_NULL;
    args2.remi.provider = remi_provider;
    yret = yk_provider_register(
            mid, 2, "{}", &args2, &provider2);
    //          ^^^^ MUST be empty!
    assert(yret == YOKAN_SUCCESS);

    // Create Yokan client
    yk_client_t client;
    yret = yk_client_init(mid, &client);
    assert(yret == YOKAN_SUCCESS);

    // Get handle to source database
    yk_database_handle_t dbh1;
    yret = yk_database_handle_create(
            client, addr, 1, true, &dbh1);
    assert(yret == YOKAN_SUCCESS);

    // Populate source database with some data
    printf("Populating source database...\n");
    for(int i = 0; i < 10; i++) {
        char key[16], value[16];
        sprintf(key, "key%05d", i);
        sprintf(value, "value%05d", i);
        yret = yk_put(dbh1, 0, key, strlen(key), value, strlen(value));
        assert(yret == YOKAN_SUCCESS);
    }
    printf("  Inserted 10 key/value pairs\n");

    // Set up migration options
    struct yk_migration_options options;
    options.new_root = "/tmp/migrated-database";
    options.extra_config = "{}";
    options.xfer_size = 0;  // Use default

    // Migrate database from provider 1 to provider 2
    printf("\nMigrating database from provider 1 to provider 2...\n");
    yret = yk_provider_migrate_database(
            provider1, addr_str, 2, &options);
    assert(yret == YOKAN_SUCCESS);
    printf("  Migration successful!\n");

    // Try to access source database - should get error now
    printf("\nVerifying source database is now invalid...\n");
    yret = yk_put(dbh1, 0, "test", 4, "data", 4);
    assert(yret == YOKAN_ERR_INVALID_DATABASE);
    printf("  Source database correctly returns INVALID_DATABASE\n");

    // Release old handle
    yk_database_handle_release(dbh1);

    // Get handle to destination database
    yk_database_handle_t dbh2;
    yret = yk_database_handle_create(
            client, addr, 2, true, &dbh2);
    assert(yret == YOKAN_SUCCESS);

    // Verify data was migrated
    printf("\nVerifying migrated data at destination...\n");
    for(int i = 0; i < 10; i++) {
        char key[16], value[16], expected[16];
        sprintf(key, "key%05d", i);
        sprintf(expected, "value%05d", i);
        size_t vsize = 16;
        yret = yk_get(dbh2, 0, key, strlen(key), value, &vsize);
        assert(yret == YOKAN_SUCCESS);
        value[vsize] = '\0';
        assert(strcmp(value, expected) == 0);
    }
    printf("  All 10 key/value pairs successfully retrieved\n");

    // Clean up
    yk_database_handle_release(dbh2);
    yk_client_finalize(client);
    remi_client_finalize(remi_client);
    margo_addr_free(mid, addr);
    margo_finalize(mid);

    printf("\nMigration example completed successfully!\n");
    return 0;
}
