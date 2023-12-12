#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <yokan/client.h>
#include <yokan/database.h>
#include <yokan/collection.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <address> <provider id>\n", argv[0]);
        exit(-1);
    }
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    uint16_t provider_id = atoi(argv[2]);
    hg_addr_t server_addr = HG_ADDR_NULL;
    hg_return_t hret = margo_addr_lookup(mid, argv[1], &server_addr);
    assert(hret == HG_SUCCESS);

    yk_return_t ret;
    yk_client_t client = YOKAN_CLIENT_NULL;

    ret = yk_client_init(mid, &client);
    assert(ret == YOKAN_SUCCESS);

    yk_database_handle_t db_handle = YOKAN_DATABASE_HANDLE_NULL;
    ret = yk_database_handle_create(
        client, server_addr, provider_id, true, &db_handle);
    assert(ret == YOKAN_SUCCESS);

    /* create a collection in the database */
    ret = yk_collection_create(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT);
    assert(ret == YOKAN_SUCCESS);

    /* check that a collection exists */
    uint8_t flag;
    ret = yk_collection_exists(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, &flag);
    assert(ret == YOKAN_SUCCESS);
    assert(flag);

    /* get the size of the collection */
    size_t size;
    ret = yk_collection_size(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, &size);
    assert(ret == YOKAN_SUCCESS);
    assert(size == 0);

    /* get the last id from the collection
     * (i.e. the id the next stored document will have)
     */
    yk_id_t last_id;
    ret = yk_collection_last_id(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, &last_id);
    assert(ret == YOKAN_SUCCESS);
    assert(size == 0);

    /* drop the collection */
    ret = yk_collection_drop(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}
