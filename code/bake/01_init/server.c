#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <bake-server.h>

int main(int argc, char** argv)
{
    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid,my_address);

    margo_set_log_level(mid, MARGO_LOG_INFO);
    margo_info(mid, "Server running at address %s", addr_str);

    bake_provider_t provider = NULL;
    struct bake_provider_init_info init_info = BAKE_PROVIDER_INIT_INFO_INITIALIZER;

    int ret = bake_provider_register(mid, 42, &init_info, &provider);
    assert(ret == BAKE_SUCCESS);

    bake_target_id_t tid;

    ret = bake_provider_create_target(
        provider, "pmem:/dev/shm/mytarget.dat",
        8388608, &tid);
    assert(ret == BAKE_SUCCESS);

    char tid_str[37];
    ret = bake_target_id_to_string(tid, tid_str, 37);
    margo_info(mid, "Target ID is %s", tid_str);

    margo_wait_for_finalize(mid);

    return 0;
}
