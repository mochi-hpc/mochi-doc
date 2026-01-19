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
        fprintf(stderr, "Usage: %s <group_file>\n", argv[0]);
        return -1;
    }

    const char* group_file = argv[1];

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

    uint16_t provider_id = 42;
    int ret;

    // Load the existing group view from file
    // This view contains the addresses of current group members
    ret = flock_group_view_init_from_file(group_file, &initial_view);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Failed to load group view from file: %s\n", group_file);
        margo_finalize(mid);
        return -1;
    }

    printf("Loaded group view from file: %s\n", group_file);
    printf("Current group size: %zu\n", initial_view.members.size);

    // Configure with centralized backend and "join" bootstrap method
    // The "join" bootstrap tells the provider to contact existing members
    // and request to join the group dynamically
    const char* config =
        "{"
        "  \"bootstrap\": \"join\","
        "  \"group\": {"
        "    \"type\": \"centralized\","
        "    \"config\": {}"
        "  }"
        "}";

    // Register provider - it will join the existing group
    flock_provider_t provider;
    ret = flock_provider_register(mid, provider_id, config, &args, &provider);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Failed to register provider and join group\n");
        flock_group_view_clear(&initial_view);
        margo_finalize(mid);
        return -1;
    }

    printf("Successfully joined the group\n");

    // Wait for finalize
    margo_wait_for_finalize(mid);

    return 0;
}
