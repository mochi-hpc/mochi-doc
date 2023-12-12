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

    const char* docs[4] = {
        "This is a document",
        "This is another one",
        "And another one",
        "And the last"};
    size_t doc_sizes[4];
    for(unsigned i = 0; i < 4; i++)
        doc_sizes[i] = strlen(docs[i]);

    /* store the documents, getting yk_id_t back */
    yk_id_t ids[4];
    ret = yk_doc_store_multi(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, 4, (const void * const*)docs,
            doc_sizes, ids);
    assert(ret == YOKAN_SUCCESS);

    /* load the documents back */
    char* buffers[4];
    size_t buf_sizes[4];
    for(unsigned i = 0; i < 4; i++) {
        buffers[i] = calloc(1, 128);
        buf_sizes[i] = 128;
    }
    ret = yk_doc_load_multi(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, 4, ids, (void * const*)buffers, buf_sizes);
    assert(ret == YOKAN_SUCCESS);

    /* get the length of a bunch of documents */
    size_t lengths[4];
    ret = yk_doc_length_multi(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, 4, ids, lengths);
    assert(ret == YOKAN_SUCCESS);

    /* update the content of the document */
    const char* updated_docs[2] = {
        "Updated first document",
        "New third document"
    };
    yk_id_t ids_to_update[2] = { 0, 2 };
    size_t updated_doc_sizes[2] = { strlen(updated_docs[0]), strlen(updated_docs[1]) };
    ret = yk_doc_update_multi(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, 2, ids_to_update,
            (const void * const*)updated_docs, updated_doc_sizes);
    assert(ret == YOKAN_SUCCESS);

    /* erase a document */
    yk_id_t ids_to_erase[3] = {0, 1, 3};
    ret = yk_doc_erase_multi(db_handle, "my_collection",
            YOKAN_MODE_DEFAULT, 3, ids_to_erase);
    assert(ret == YOKAN_SUCCESS);

    /* list documents */
    buf_sizes[0] = 128;
    buf_sizes[1] = 128;
    yk_id_t listed_ids[2];
    ret = yk_doc_list(db_handle, "my_collection",
            YOKAN_MODE_INCLUSIVE, 1, NULL, 0, 2,
            listed_ids, (void * const*)buffers, buf_sizes);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}
