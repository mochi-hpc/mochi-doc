/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <assert.h>
#include <flock/flock-client.h>
#include <flock/flock-group.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server address> <provider id>\n", argv[0]);
        exit(-1);
    }

    flock_return_t ret;
    const char* svr_addr_str = argv[1];
    uint16_t provider_id = atoi(argv[2]);

    /* Initialize Margo in client mode */
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    margo_set_log_level(mid, MARGO_LOG_INFO);

    /* Lookup server address */
    hg_addr_t svr_addr;
    margo_addr_lookup(mid, svr_addr_str, &svr_addr);

    /* Create Flock client */
    flock_client_t client;
    ret = flock_client_init(mid, ABT_POOL_NULL, &client);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "flock_client_init failed\n");
        return -1;
    }

    /* Create group handle */
    flock_group_handle_t group_handle;
    ret = flock_group_handle_create(client, svr_addr, provider_id, true, &group_handle);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "flock_group_handle_create failed\n");
        return -1;
    }

    /* Query group membership */
    flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
    ret = flock_group_get_view(group_handle, &view);
    if(ret != FLOCK_SUCCESS) {
        fprintf(stderr, "flock_group_handle_get_view failed\n");
        return -1;
    }

    printf("Group has %zu members:\n", view.members.size);
    for(size_t i = 0; i < view.members.size; i++) {
        printf("  [%zu] %s (provider_id=%d)\n",
               i, view.members.data[i].address,
               view.members.data[i].provider_id);
    }

    /* Clean up */
    flock_group_view_clear(&view);
    flock_group_handle_release(group_handle);
    flock_client_finalize(client);
    margo_addr_free(mid, svr_addr);
    margo_finalize(mid);

    return 0;
}
