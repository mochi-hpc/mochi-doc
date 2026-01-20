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

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server address> <provider id>\n", argv[0]);
        exit(-1);
    }

    flock_return_t ret;
    hg_return_t hret;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);

    /* Initialize Margo in client mode */
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    margo_set_log_level(mid, MARGO_LOG_INFO);

    /* Lookup server address */
    hg_addr_t svr_addr;
    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid, "margo_addr_lookup failed for address %s", svr_addr_str);
    }

    /* Create Flock client */
    flock_client_t flock_clt;
    margo_info(mid, "Creating FLOCK client");
    ret = flock_client_init(mid, ABT_POOL_NULL, &flock_clt);
    if(ret != FLOCK_SUCCESS) {
        FATAL(mid, "flock_client_init failed (ret = %d)", ret);
    }

    /* Create group handle */
    flock_group_handle_t flock_gh;
    margo_info(mid, "Creating group handle for provider id %d", (int)provider_id);
    ret = flock_group_handle_create(flock_clt, svr_addr, provider_id, true, &flock_gh);
    if(ret != FLOCK_SUCCESS) {
        FATAL(mid, "flock_group_handle_create failed (ret = %d)", ret);
    }

    /* Use the group handle (see later tutorials) */
    margo_info(mid, "Group handle created successfully");

    /* Release group handle */
    margo_info(mid, "Releasing group handle");
    ret = flock_group_handle_release(flock_gh);
    if(ret != FLOCK_SUCCESS) {
        FATAL(mid, "flock_group_handle_release failed (ret = %d)", ret);
    }

    /* Finalize client */
    margo_info(mid, "Finalizing client");
    ret = flock_client_finalize(flock_clt);
    if(ret != FLOCK_SUCCESS) {
        FATAL(mid, "flock_client_finalize failed (ret = %d)", ret);
    }

    /* Free address and finalize */
    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid, "Could not free address (margo_addr_free returned %d)", hret);
    }

    margo_finalize(mid);

    return 0;
}
