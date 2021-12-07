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
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <address> <provider id> <database id>\n", argv[0]);
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
    yk_database_id_t db_id;

    ret = yk_client_init(mid, &client);
    assert(ret == YOKAN_SUCCESS);

    yk_database_id_from_string(argv[3], &db_id);

    yk_database_handle_t db_handle = YOKAN_DATABASE_HANDLE_NULL;
    ret = yk_database_handle_create(
        client, server_addr, provider_id, db_id, &db_handle);
    assert(ret == YOKAN_SUCCESS);

    /* create a collection in the database */
    ret = yk_collection_create(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT);
    assert(ret == YOKAN_SUCCESS);

    const char* document = "This is a document";
    size_t doc_size = strlen(document);

    /* store the document, getting a yk_id_t back */
    yk_id_t id;
    ret = yk_doc_store(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, document, doc_size, &id);
    assert(ret == YOKAN_SUCCESS);
    printf("Document has id %lu\n", id);

    /* load the document back */
    char buffer[128];
    memset(buffer, 0, 128);
    size_t buf_size = 128;
    ret = yk_doc_load(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, id, buffer, &buf_size);
    assert(ret == YOKAN_SUCCESS);
    assert(strcmp(document, buffer) == 0);

    /* get the length of a document */
    size_t length;
    ret = yk_doc_length(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, id, &length);
    assert(ret == YOKAN_SUCCESS);
    assert(length == doc_size);

    /* update the content of the document */
    const char* updated_document = "Updated document";
    doc_size = strlen(updated_document);
    ret = yk_doc_update(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, id, updated_document, doc_size);
    assert(ret == YOKAN_SUCCESS);

    /* erase a document */
    ret = yk_doc_erase(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, id);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}
