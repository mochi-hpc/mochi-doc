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

    /* Initialize Margo */
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "Failed to initialize Margo\n");
        return -1;
    }

    /* Create Warabi client */
    warabi_client_t client;
    ret = warabi_client_create(mid, &client);
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create client\n");
        margo_finalize(mid);
        return -1;
    }

    /* Create target handle */
    warabi_target_handle_t target;
    ret = warabi_client_make_target_handle(client, server_addr, provider_id, &target);
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create target handle\n");
        warabi_client_free(client);
        margo_finalize(mid);
        return -1;
    }

    /* Create a region */
    warabi_region_t region;
    ret = warabi_create(target, 1024, &region, WARABI_ASYNC_REQUEST_IGNORE);
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to create region\n");
        goto cleanup;
    }

    /* Async write example */
    const char* data = "Async write test";
    warabi_async_request_t write_req;

    printf("Issuing async write...\n");
    ret = warabi_write(
        target,
        region,
        0,              /* offset */
        data,
        strlen(data),
        false,          /* persist */
        &write_req      /* Async request */
    );

    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to issue async write\n");
        goto cleanup;
    }

    /* Do other work while write is in progress */
    printf("Write in progress, doing other work...\n");

    /* Wait for write to complete */
    ret = warabi_wait(write_req);
    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Async write failed\n");
        goto cleanup;
    }
    printf("Async write completed\n");

    /* Async read example */
    char buffer[1024];
    warabi_async_request_t read_req;

    printf("Issuing async read...\n");
    ret = warabi_read(
        target,
        region,
        0,              /* offset */
        buffer,
        strlen(data),
        &read_req       /* Async request */
    );

    if(ret != WARABI_SUCCESS) {
        fprintf(stderr, "Failed to issue async read\n");
        goto cleanup;
    }

    /* Test for completion */
    bool completed = false;
    int iterations = 0;

    while(!completed) {
        ret = warabi_test(read_req, &completed);
        if(completed) {
            printf("Async read completed after %d checks\n", iterations);
        } else {
            iterations++;
            /* Do other work... */
        }
    }

    buffer[strlen(data)] = '\0';
    printf("Read: %s\n", buffer);

    /* Cleanup */
    warabi_erase(target, region, WARABI_ASYNC_REQUEST_IGNORE);

cleanup:
    warabi_target_handle_free(target);
    warabi_client_free(client);
    margo_finalize(mid);

    return (ret == WARABI_SUCCESS) ? 0 : -1;
}
