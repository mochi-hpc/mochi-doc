#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <bake-client.h>

int main(int argc, char** argv)
{
    int ret;

    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    assert(argc == 2 || argc == 3);

    const char* server_addr_str = argv[1];
    const char* target_id_str = argc == 3 ? argv[2] : NULL;

    hg_addr_t server_address;
    hg_return_t hret = margo_addr_lookup(mid, server_addr_str, &server_address);
    assert(hret == HG_SUCCESS);

    bake_client_t client;
    ret = bake_client_init(mid, &client);
    assert(ret == 0);

    bake_provider_handle_t ph;
    ret = bake_provider_handle_create(client, server_address, 42, &ph);
    assert(ret == 0);

    bake_target_id_t tid;
    if(target_id_str) {
        ret = bake_target_id_from_string(target_id_str, &tid);
        assert(ret == 0);
    } else {
        uint64_t num_targets;
        ret = bake_probe(ph, 1, &tid, &num_targets);
        assert(ret == 0);
        assert(num_targets == 1);
    }

    bake_region_id_t rid;
    ret = bake_create(ph, tid, 10, &rid);
    assert(ret == 0);

    char in_buffer[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
    ret = bake_write(ph, tid, rid, 0, in_buffer, 10);
    assert(ret == 0);

    ret = bake_persist(ph, tid, rid, 0, 10);
    assert(ret == 0);

    char out_buffer[10];
    uint64_t bytes_read;
    ret = bake_read(ph, tid, rid, 0, out_buffer, 10, &bytes_read);
    assert(ret == 0);
    assert(memcmp(in_buffer, out_buffer, 10) == 0);

    ret = bake_provider_handle_release(ph);
    assert(ret == 0);

    ret = bake_client_finalize(client);
    assert(ret == 0);

    hret = margo_addr_free(mid, server_address);
    assert(hret == HG_SUCCESS);

    margo_finalize(mid);

    return 0;
}
