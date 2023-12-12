#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <yokan/client.h>
#include <yokan/database.h>

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

    ret = yk_put(db_handle, YOKAN_MODE_DEFAULT,
                 key, strlen(key), value_in, strlen(value_in));
    assert(ret == YOKAN_SUCCESS);

    char value_out[128];
    size_t value_out_size = 128;
    ret = yk_get(db_handle, YOKAN_MODE_DEFAULT,
                 key, strlen(key), value_out, &value_out_size);
    assert(ret == YOKAN_SUCCESS);

    assert(strcmp(value_in, value_out) == 0);

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}
