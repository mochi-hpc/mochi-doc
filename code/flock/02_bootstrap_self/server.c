/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <flock/flock-server.h>
#include <flock/flock-bootstrap.h>

int main(int argc, char** argv)
{
    // Initialize Margo
    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    // Print server address
    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[256];
    hg_size_t addr_str_size = 256;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid, my_address);

    printf("Server running at address %s\n", addr_str);

    // Initialize provider args
    struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
    flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
    args.initial_view = &initial_view;

    // Bootstrap using self method
    uint16_t provider_id = 42;
    flock_group_view_init_from_self(mid, provider_id, &initial_view);

    printf("Bootstrapped group with self method\n");
    printf("Group size: %zu\n", initial_view.members.size);

    // Configure with static backend
    const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";

    // Register provider
    flock_provider_t provider;
    int ret = flock_provider_register(mid, provider_id, config, &args, &provider);
    assert(ret == FLOCK_SUCCESS);

    printf("Flock provider registered with provider_id=%d\n", provider_id);

    // Wait for finalize
    margo_wait_for_finalize(mid);

    return 0;
}
