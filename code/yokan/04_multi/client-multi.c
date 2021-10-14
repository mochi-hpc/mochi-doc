#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <yokan/client.h>
#include <yokan/database.h>

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <address> <provider id> <database id>\n", argv[0]);
        exit(-1);
    }
    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    uint16_t provider_id = atoi(argv[2]);
    hg_addr_t server_addr = HG_ADDR_NULL;
    hg_return_t hret = margo_addr_lookup(mid, argv[1], &server_addr);
    assert(hret == HG_SUCCESS);

    yk_return_t ret;
    yk_client_t client = YOKAN_CLIENT_NULL;
    yk_database_id_t db_id;

    ret = yk_client_init(mid, &client);
    assert(ret == YOKAN_SUCCESS);

    yk_database_id_from_string(argv[3], &db_id);

    yk_database_handle_t db_handle = YOKAN_DATABASE_HANDLE_NULL;
    ret = yk_database_handle_create(
        client, server_addr, provider_id, db_id, &db_handle);
    assert(ret == YOKAN_SUCCESS);

    // ------------------------------------------------------
    printf("Putting the following key/value pairs:\n");
    const char* const keys[] = {
        "matthieu",
        "phil",
        "rob",
        "shane",
        "kevin",
        "zhe",
        "srinivasan"
    };
    const char* const values[] = {
        "dorier",
        "carns",
        "ross",
        "snyder",
        "harms",
        "wang",
        "ramesh"
    };
    size_t ksizes[7];
    size_t vsizes[7];
    for(int i = 0; i < 7; i++) {
        ksizes[i] = strlen(keys[i]);
        vsizes[i] = strlen(values[i]);
        printf("\t%s => %s\n", keys[i], values[i]);
    }

    /* putting multiple key/value pairs at once */
    ret = yk_put_multi(db_handle, YOKAN_MODE_DEFAULT, 7,
                       (const void* const*)keys, ksizes,
                       (const void* const*)values, vsizes);
    assert(ret == YOKAN_SUCCESS);

    // ------------------------------------------------------

    const char* const keys2[4] = {
        "matthieu",
        "marc",
        "anna",
        "shane",
    };
    size_t ksizes2[4];
    for(int i=0; i < 4; i++)
        ksizes2[i] = strlen(keys2[i]);

    /* checking that the keys exist */
    uint8_t flags[1];
    ret = yk_exists_multi(db_handle, YOKAN_MODE_DEFAULT, 4,
                          (const void* const*)keys2, ksizes2, flags);
    assert(ret == YOKAN_SUCCESS);
    printf("Checking if the following keys exist:\n");
    for(int i=0; i < 4; i++) {
        bool flag = yk_unpack_exists_flag(flags, i);
        printf("\t%s => %s\n", keys2[i], flag ? "YES" : "NO");
    }

    // ------------------------------------------------------

    size_t vsizes2[4];

    /* getting the length of values associated with keys */
    ret = yk_length_multi(db_handle, YOKAN_MODE_DEFAULT, 4,
                          (const void* const*)keys2, ksizes2, vsizes2);
    assert(ret == YOKAN_SUCCESS);
    printf("Checking if the length of the values for the following keys:\n");
    for(int i=0; i < 4; i++) {
        if(vsizes2[i] != YOKAN_KEY_NOT_FOUND)
            printf("\t%s => %lu\n", keys2[i], vsizes2[i]);
        else
            printf("\t%s => (not found)\n", keys2[i]);
    }

    // ------------------------------------------------------

    char* values2[4];
    for(int i = 0; i < 4; i++) {
        values2[i] = calloc(1, 16);
        vsizes2[i] = 16;
    }
    // intentionally limiting the size of the last key
    vsizes2[3] = 3;

    /* getting multiple key/value pairs at once */
    ret = yk_get_multi(db_handle, YOKAN_MODE_DEFAULT, 4,
                       (const void* const*)keys2, ksizes2,
                       (void * const*)values2, vsizes2);
    assert(ret == YOKAN_SUCCESS);
    printf("Getting the values for the following keys:\n");
    for(int i=0; i < 4; i++) {
        if(vsizes2[i] == YOKAN_KEY_NOT_FOUND)
            printf("\t%s => (not found)\n", keys2[i]);
        else if(vsizes2[i] == YOKAN_SIZE_TOO_SMALL)
            printf("\t%s => (buffer too small)\n", keys2[i]);
        else
            printf("\t%s => %s\n", keys2[i], values2[i]);
    }

    for(int i = 0; i < 4; i++) {
        free(values2[i]);
    }

    // ------------------------------------------------------

    char* keys3[4];
    size_t ksizes3[4];
    for(int i = 0; i < 4; i++) {
        keys3[i] = calloc(1, 8);
        ksizes3[i] = 8; // 8 won't be enough for "srinivasan"
    }

    ret = yk_list_keys(db_handle, YOKAN_MODE_INCLUSIVE,
            "shane", 5, "", 0, 4, (void* const*)keys3, ksizes3);
    assert(ret == YOKAN_SUCCESS);
    printf("Listed the following keys:\n");
    for(int i=0; i < 4; i++) {
        if(ksizes3[i] == YOKAN_NO_MORE_KEYS)
            break;
        else if(ksizes3[i] == YOKAN_SIZE_TOO_SMALL)
            printf("\t(buffer too small)\n");
        else
            printf("\t%s\n", keys3[i]);
    }

    for(int i=0; i < 4; i++)
        free(keys3[i]);

    // ------------------------------------------------------

    char* values3[4];
    size_t vsizes3[4];
    for(int i = 0; i < 4; i++) {
        values3[i] = calloc(1, 16);
        vsizes3[i] = 16;
        keys3[i] = calloc(1, 16);
        ksizes3[i] = 16;
    }

    ret = yk_list_keyvals(db_handle, YOKAN_MODE_INCLUSIVE,
            "shane", 5, "", 0, 4, (void* const*)keys3, ksizes3, (void* const*)values3, vsizes3);
    assert(ret == YOKAN_SUCCESS);
    printf("Listed the following key/value pairs:\n");
    for(int i=0; i < 4; i++) {
        if(ksizes3[i] == YOKAN_NO_MORE_KEYS)
            break;
        values3[i][vsizes3[i]] = '\0';
        printf("\t%s => %s\n", keys3[i], values3[i]);
    }

    for(int i=0; i < 4; i++) {
        free(keys3[i]);
        free(values3[i]);
    }

    // ------------------------------------------------------

    printf("Erasing all the key/value pairs.\n");
    ret = yk_erase_multi(db_handle, YOKAN_MODE_DEFAULT, 7,
                         (const void * const*)keys, ksizes);
    assert(ret == YOKAN_SUCCESS);

    // ------------------------------------------------------

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}
