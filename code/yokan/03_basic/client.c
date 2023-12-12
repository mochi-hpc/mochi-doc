#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <yokan/client.h>
#include <yokan/database.h>

static yk_return_t keyvalue_callback(void*, size_t, const void*, size_t, const void*, size_t);

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

    const char* key = "matthieu";
    const char* value_in = "dorier";

    /* putting a key/value pair */
    ret = yk_put(db_handle, YOKAN_MODE_DEFAULT,
                 key, strlen(key), value_in, strlen(value_in));
    assert(ret == YOKAN_SUCCESS);

    /* getting the current number of stored key/value pairs */
    size_t count;
    ret = yk_count(db_handle, YOKAN_MODE_DEFAULT, &count);

    /* checking that the key exists */
    uint8_t flag;
    ret = yk_exists(db_handle, YOKAN_MODE_DEFAULT,
                    key, strlen(key), &flag);
    assert(ret == YOKAN_SUCCESS);
    assert(flag);

    /* getting the length of the value associated with the key */
    size_t vsize;
    ret = yk_length(db_handle, YOKAN_MODE_DEFAULT,
                    key, strlen(key), &vsize);

    /* getting the value associated with a key */
    char* value_out = malloc(vsize);
    size_t value_out_size = vsize;
    ret = yk_get(db_handle, YOKAN_MODE_DEFAULT,
                 key, strlen(key), value_out, &value_out_size);
    assert(ret == YOKAN_SUCCESS);
    free(value_out);

    /* getting the value associated with a key using a callback */
    ret = yk_fetch(db_handle, YOKAN_MODE_DEFAULT,
                   key, strlen(key), keyvalue_callback, NULL);
    assert(ret == YOKAN_SUCCESS);

    /* deleting a key/value pair */
    ret = yk_erase(db_handle, YOKAN_MODE_DEFAULT, key, strlen(key));

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}

static yk_return_t keyvalue_callback(
        void* uargs, size_t i,
        const void* key, size_t ksize,
        const void* val, size_t vsize) {
    (void)uargs;
    (void)i;
    printf("Inside callback: %.*s => %.*s\n",
           (int)ksize, (const char*)key, (int)vsize, (const char*)val);
}
