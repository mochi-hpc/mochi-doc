#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <yokan/server.h>

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

    yk_return_t ret = yk_provider_register(mid, 42, NULL, YOKAN_PROVIDER_IGNORE);
    assert(ret == YOKAN_SUCCESS);

    margo_wait_for_finalize(mid);

    return 0;
}
