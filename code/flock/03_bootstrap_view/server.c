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
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <view_file>\n", argv[0]);
        return -1;
    }

    const char* view_file = argv[1];

    // Initialize Margo
    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    // Initialize provider args
    struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
    flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
    args.initial_view = &initial_view;

    // Bootstrap from view file
    uint16_t provider_id = 42;
    int ret = flock_group_view_init_from_file(view_file, &initial_view);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Failed to initialize view from file: %s\n", view_file);
        margo_finalize(mid);
        return -1;
    }

    printf("Bootstrapped group from view file: %s\n", view_file);
    printf("Group size: %zu\n", initial_view.members.size);

    // Print all members
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
