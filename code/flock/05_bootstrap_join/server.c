/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <margo.h>
#include <flock/flock-server.h>
#include <flock/flock-bootstrap.h>
#include <flock/flock-client.h>
#include <flock/flock-group.h>

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <bootstrap_address>\n", argv[0]);
        fprintf(stderr, "  Use 'self' for the first server\n");
        return -1;
    }

    const char* bootstrap_addr = argv[1];

    // Initialize Margo (in server mode, but with client capability for join)
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

    // Bootstrap using self or join method
    if(strcmp(bootstrap_addr, "self") == 0) {
        // First server: bootstrap with self
        flock_group_view_init_from_self(mid, provider_id, &initial_view);
        printf("Bootstrapped as initial member (self)\n");
    } else {
        // Subsequent servers: join existing group by getting its view
        // Create a flock client to communicate with the existing group
        flock_client_t client;
        ret = flock_client_init(mid, ABT_POOL_NULL, &client);
        if(ret != FLOCK_SUCCESS) {
            fprintf(stderr, "Failed to create flock client\n");
            margo_finalize(mid);
            return -1;
        }

        // Lookup the bootstrap server address
        hg_addr_t bootstrap_hg_addr;
        ret = margo_addr_lookup(mid, bootstrap_addr, &bootstrap_hg_addr);
        if(ret != HG_SUCCESS) {
            fprintf(stderr, "Failed to lookup address %s\n", bootstrap_addr);
            flock_client_finalize(client);
            margo_finalize(mid);
            return -1;
        }

        // Create group handle to get the current view
        flock_group_handle_t group_handle;
        ret = flock_group_handle_create(client, bootstrap_hg_addr, provider_id, 0, &group_handle);
        if(ret != FLOCK_SUCCESS) {
            fprintf(stderr, "Failed to create group handle\n");
            margo_addr_free(mid, bootstrap_hg_addr);
            flock_client_finalize(client);
            margo_finalize(mid);
            return -1;
        }

        // Get the current view from the group
        ret = flock_group_get_view(group_handle, &initial_view);
        if(ret != FLOCK_SUCCESS) {
            fprintf(stderr, "Failed to get group view\n");
            flock_group_handle_release(group_handle);
            margo_addr_free(mid, bootstrap_hg_addr);
            flock_client_finalize(client);
            margo_finalize(mid);
            return -1;
        }

        // Add ourselves to the view
        flock_group_view_add_member(&initial_view, addr_str, provider_id);

        printf("Joined existing group via %s\n", bootstrap_addr);

        // Clean up client resources
        flock_group_handle_release(group_handle);
        margo_addr_free(mid, bootstrap_hg_addr);
        flock_client_finalize(client);
    }

    printf("Group size: %zu\n", initial_view.members.size);

    // Configure with centralized backend (supports dynamic membership)
    const char* config = "{ \"group\":{ \"type\":\"centralized\", \"config\":{} } }";

    // Register provider
    flock_provider_t provider;
    ret = flock_provider_register(mid, provider_id, config, &args, &provider);
    assert(ret == FLOCK_SUCCESS);

    printf("Flock provider registered\n");

    // Wait for finalize
    margo_wait_for_finalize(mid);

    return 0;
}
