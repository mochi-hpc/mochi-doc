/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <warabi/client.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <server_addr> <provider_id>\n", argv[0]);
        return -1;
    }

    const char* server_addr = argv[1];
    uint16_t provider_id = (uint16_t)atoi(argv[2]);

    warabi_err_t ret;
    margo_instance_id mid = MARGO_INSTANCE_NULL;
    hg_addr_t addr = HG_ADDR_NULL;
    warabi_client_t client = WARABI_CLIENT_NULL;
    warabi_target_handle_t target = WARABI_TARGET_HANDLE_NULL;

    /* Initialize Margo */
    mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "Failed to initialize Margo\n");
        return -1;
    }
    printf("Client initialized\n");

    /* Create Warabi client */
    ret = warabi_client_create(mid, &client);
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create Warabi client\n");
        goto cleanup;
    }

    /* Look up server address */
    hg_return_t hret = margo_addr_lookup(mid, server_addr, &addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Failed to lookup server address\n");
        goto cleanup;
    }

    /* Create target handle */
    ret = warabi_client_make_target_handle(
        client,
        server_addr,
        provider_id,
        &target
    );
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create target handle\n");
        goto cleanup;
    }
    printf("Connected to target\n");

    /* Create a region */
    warabi_region_t region;
    size_t region_size = 1024;

    ret = warabi_create(
        target,
        region_size,
        &region,
        WARABI_ASYNC_REQUEST_IGNORE
    );
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create region\n");
        goto cleanup;
    }
    printf("Created region\n");

    /* Write data to the region */
    const char* message = "Hello, Warabi!";
    size_t message_size = strlen(message);

    ret = warabi_write(
        target,
        region,
        0,           /* offset */
        message,
        message_size,
        false,       /* persist */
        WARABI_ASYNC_REQUEST_IGNORE
    );
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to write data\n");
        goto cleanup_region;
    }
    printf("Wrote %zu bytes\n", message_size);

    /* Read data back */
    char buffer[1024];
    size_t buffer_size = message_size;

    ret = warabi_read(
        target,
        region,
        0,           /* offset */
        buffer,
        buffer_size,
        WARABI_ASYNC_REQUEST_IGNORE
    );
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to read data\n");
        goto cleanup_region;
    }
    printf("Read: %.*s\n", (int)buffer_size, buffer);

    /* Verify data */
    if(memcmp(buffer, message, message_size) == 0) {
        printf("SUCCESS: Data verified\n");
    } else {
        printf("ERROR: Data mismatch\n");
    }

cleanup_region:
    /* Erase the region */
    ret = warabi_erase(target, region, WARABI_ASYNC_REQUEST_IGNORE);
    if(ret == WARABI_SUCCESS) {
        printf("Region erased\n");
    }

cleanup:
    /* Free resources */
    if(target != WARABI_TARGET_HANDLE_NULL)
        warabi_target_handle_free(target);
    if(client != WARABI_CLIENT_NULL)
        warabi_client_free(client);
    if(addr != HG_ADDR_NULL)
        margo_addr_free(mid, addr);
    if(mid != MARGO_INSTANCE_NULL)
        margo_finalize(mid);

    return (ret == WARABI_SUCCESS) ? 0 : -1;
}
