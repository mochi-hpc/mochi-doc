/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <warabi/server.h>

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <protocol>\n", argv[0]);
        return -1;
    }

    /* Initialize Margo */
    margo_instance_id mid = margo_init(argv[1], MARGO_SERVER_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "Failed to initialize Margo\n");
        return -1;
    }

    /* Warabi configuration (JSON) */
    const char* config = "{"
        "\"target\": "
            "{\"type\": \"memory\", \"config\": {}}"
    "}";

    /* Register Warabi provider */
    warabi_provider_t provider;
    warabi_err_t ret = warabi_provider_register(
        &provider,              /* Output provider */
        mid,                    /* Margo instance */
        42,                     /* Provider ID */
        config,                 /* Configuration */
        NULL                    /* Default init args */
    );

    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create provider\n");
        margo_finalize(mid);
        return -1;
    }

    /* Print server address */
    hg_addr_t self_addr;
    margo_addr_self(mid, &self_addr);
    char self_addr_str[128];
    hg_size_t self_addr_size = 128;
    margo_addr_to_string(mid, self_addr_str, &self_addr_size, self_addr);
    margo_addr_free(mid, self_addr);

    printf("Warabi server running at: %s\n", self_addr_str);
    printf("Provider ID: 42\n");

    /* Wait for finalize */
    margo_wait_for_finalize(mid);

    /* Cleanup */
    warabi_provider_deregister(provider);
    margo_finalize(mid);

    return 0;
}
