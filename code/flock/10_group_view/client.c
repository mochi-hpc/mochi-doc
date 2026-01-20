/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <flock/flock-client.h>
#include <flock/flock-group.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server_address> <provider_id>\n", argv[0]);
        return -1;
    }

    const char* server_addr_str = argv[1];
    uint16_t provider_id = atoi(argv[2]);

    // Initialize Margo
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    // Initialize Flock client
    flock_client_t client;
    int ret = flock_client_init(mid, ABT_POOL_NULL, &client);
    assert(ret == FLOCK_SUCCESS);

    // Lookup server address
    hg_addr_t server_addr;
    ret = margo_addr_lookup(mid, server_addr_str, &server_addr);
    if(ret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup server address\n");
        flock_client_finalize(client);
        margo_finalize(mid);
        return -1;
    }

    // Create group handle
    flock_group_handle_t group;
    ret = flock_group_handle_create(client, server_addr, provider_id, 0, &group);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Failed to create group handle\n");
        margo_addr_free(mid, server_addr);
        flock_client_finalize(client);
        margo_finalize(mid);
        return -1;
    }

    printf("Connected to Flock group\n");

    // Get current group view
    flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
    ret = flock_group_get_view(group, &view);
    assert(ret == FLOCK_SUCCESS);

    printf("\n=== Current Group View ===\n");
    printf("Group size: %zu members\n", view.members.size);
    printf("View digest: %lu\n", (unsigned long)view.digest);

    // Print metadata if any
    if(view.metadata.size > 0) {
        printf("Metadata (%zu entries):\n", view.metadata.size);
        for(size_t i = 0; i < view.metadata.size; i++) {
            printf("  %s = %s\n",
                   view.metadata.data[i].key,
                   view.metadata.data[i].value);
        }
    }

    // Print all members
    printf("\nGroup members:\n");
    for(size_t i = 0; i < view.members.size; i++) {
        printf("  [%zu] Address: %s\n", i, view.members.data[i].address);
        printf("      Provider ID: %u\n", view.members.data[i].provider_id);
    }

    // Get number of members using the view helper function
    size_t num_members = flock_group_view_member_count(&view);
    printf("\nNumber of members (via helper): %zu\n", num_members);

    // Clean up view
    flock_group_view_clear(&view);

    // Release group handle
    flock_group_handle_release(group);

    // Free server address
    margo_addr_free(mid, server_addr);

    // Finalize client
    flock_client_finalize(client);

    // Finalize Margo
    margo_finalize(mid);

    printf("\nGroup view operations completed successfully\n");

    return 0;
}
