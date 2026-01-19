/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <margo.h>
#include <flock/flock-server.h>
#include <flock/flock-bootstrap.h>

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <group_file>\n", argv[0]);
        return -1;
    }

    const char* group_file = argv[1];

    // Initialize Margo
    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    // Get server address
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

    uint16_t provider_id = 42;
    int ret;

    // Bootstrap from file: read existing group view from a JSON file
    ret = flock_group_view_init_from_file(group_file, &initial_view);

    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Failed to bootstrap from file %s\n", group_file);
        margo_finalize(mid);
        return -1;
    }

    printf("Bootstrapped group from file: %s\n", group_file);
    printf("Group size: %zu\n", initial_view.members.size);

    // Print all members in the initial view
    for(size_t i = 0; i < initial_view.members.size; i++) {
        printf("  Member %zu: %s (provider_id=%u)\n",
               i,
               initial_view.members.data[i].address,
               initial_view.members.data[i].provider_id);
    }

    // Configure with static backend
    const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";

    // Register provider
    flock_provider_t provider;
    ret = flock_provider_register(mid, provider_id, config, &args, &provider);
    assert(ret == FLOCK_SUCCESS);

    printf("Flock provider registered\n");

    // Wait for finalize
    margo_wait_for_finalize(mid);

    return 0;
}
