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
    (void)argc;
    (void)argv;

    /* Initialize Margo */
    margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    margo_set_log_level(mid, MARGO_LOG_INFO);

    /* Get and print server address */
    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid, my_address);
    margo_info(mid, "Server running at address %s, with provider id 42", addr_str);

    /* Initialize provider arguments and group view */
    struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
    flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
    args.initial_view = &initial_view;

    /* Initialize the view from self (single-member group) */
    flock_group_view_init_from_self(mid, 42, &initial_view);

    /* Configuration for static backend */
    const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";

    /* Register the provider */
    flock_provider_register(mid, 42, config, &args, FLOCK_PROVIDER_IGNORE);

    /* Wait for finalize */
    margo_wait_for_finalize(mid);

    return 0;
}
