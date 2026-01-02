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

    // Initialize provider args
    struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
    flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
    args.initial_view = &initial_view;

    // Bootstrap using self
    uint16_t provider_id = 42;
    flock_group_view_init_from_self(mid, provider_id, &initial_view);

    // Configure with static backend
    // Static backend: group membership cannot change after initialization
    const char* config =
        "{"
        "  \"group\": {"
        "    \"type\": \"static\","
        "    \"config\": {}"
        "  }"
        "}";

    // Register provider with static backend
    flock_provider_t provider;
    int ret = flock_provider_register(mid, provider_id, config, &args, &provider);
    assert(ret == FLOCK_SUCCESS);

    printf("Flock provider registered with STATIC backend\n");
    printf("Group membership is fixed and cannot change\n");
    printf("Initial group size: %zu\n", initial_view.members.size);

    // Wait for finalize
    margo_wait_for_finalize(mid);

    return 0;
}
