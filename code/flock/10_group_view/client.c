/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <flock/flock-client.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server_address> <provider_id>\n", argv[0]);
        return -1;
    }

    const char* server_addr = argv[1];
    uint16_t provider_id = atoi(argv[2]);

    // Initialize Margo
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    // Initialize Flock client
    flock_client_t client;
    int ret = flock_client_init(mid, &client);
    assert(ret == FLOCK_SUCCESS);

    // Create group handle
    flock_group_handle_t group;
    ret = flock_group_handle_create(client, server_addr, provider_id, &group);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "Failed to create group handle\n");
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
    printf("View digest: %lu\n", view.digest);
    if(view.metadata) {
        printf("Metadata: %s\n", view.metadata);
    }

    // Print all members
    printf("\nGroup members:\n");
    for(size_t i = 0; i < view.members.size; i++) {
        printf("  [%zu] Rank: %zu\n", i, view.members.data[i].rank);
        printf("      Address: %s\n", view.members.data[i].address);
        printf("      Provider ID: %d\n", view.members.data[i].provider_id);
        if(view.members.data[i].metadata) {
            printf("      Metadata: %s\n", view.members.data[i].metadata);
        }
    }

    // Get number of members
    size_t num_members;
    ret = flock_group_get_num_members(group, &num_members);
    assert(ret == FLOCK_SUCCESS);
    printf("\nNumber of members (via API): %zu\n", num_members);

    // Clean up view
    flock_group_view_clear(&view);

    // Release group handle
    flock_group_handle_release(group);

    // Finalize client
    flock_client_finalize(client);

    // Finalize Margo
    margo_finalize(mid);

    printf("\nGroup view operations completed successfully\n");

    return 0;
}
