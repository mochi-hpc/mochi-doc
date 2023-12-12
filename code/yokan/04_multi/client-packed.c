#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <yokan/client.h>
#include <yokan/database.h>

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <address> <provider id>\n", argv[0]);
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

    ret = yk_client_init(mid, &client);
    assert(ret == YOKAN_SUCCESS);

    yk_database_handle_t db_handle = YOKAN_DATABASE_HANDLE_NULL;
    ret = yk_database_handle_create(
        client, server_addr, provider_id, true, &db_handle);
    assert(ret == YOKAN_SUCCESS);

    // ------------------------------------------------------
    printf("Putting the following key/value pairs:\n");
    const char* keys =
        "matthieuphilrobshanekevinzhesrinivasan";
    const char* values =
        "doriercarnsrosssnyderharmswangramesh";
    size_t ksizes[] = { 8, 4, 3, 5, 5, 3, 10 };
    size_t vsizes[] = { 6, 5, 4, 6, 5, 4, 6 };

    size_t koffset = 0;
    size_t voffset = 0;
    for(int i = 0; i < 7; i++) {
        printf("\t%.*s => %.*s\n", (int)ksizes[i], keys+koffset,
                                   (int)vsizes[i], values+voffset);
        koffset += ksizes[i];
        voffset += vsizes[i];
    }

    /* putting multiple key/value pairs at once */
    ret = yk_put_packed(db_handle, YOKAN_MODE_DEFAULT, 7,
                       (const void*)keys, ksizes,
                       (const void*)values, vsizes);
    assert(ret == YOKAN_SUCCESS);

    // ------------------------------------------------------

    const char* keys2 = "matthieumarcannashane";
    size_t ksizes2[] = { 8, 4, 4, 5};

    /* checking that the keys exist */
    uint8_t flags[1];
    ret = yk_exists_packed(db_handle, YOKAN_MODE_DEFAULT, 4,
                          (const void*)keys2, ksizes2, flags);
    assert(ret == YOKAN_SUCCESS);
    printf("Checking if the following keys exist:\n");
    koffset = 0;
    for(int i=0; i < 4; i++) {
        bool flag = yk_unpack_exists_flag(flags, i);
        printf("\t%.*s => %s\n", (int)ksizes2[i], keys2+koffset, flag ? "YES" : "NO");
        koffset += ksizes2[i];
    }

    // ------------------------------------------------------

    size_t vsizes2[4];

    /* getting the length of values associated with keys */
    ret = yk_length_packed(db_handle, YOKAN_MODE_DEFAULT, 4,
                          (const void*)keys2, ksizes2, vsizes2);
    assert(ret == YOKAN_SUCCESS);
    printf("Checking if the length of the values for the following keys:\n");
    koffset = 0;
    for(int i=0; i < 4; i++) {
        if(vsizes2[i] != YOKAN_KEY_NOT_FOUND)
            printf("\t%.*s => %lu\n", (int)ksizes2[i], keys2+koffset, vsizes2[i]);
        else
            printf("\t%.*s => (not found)\n", (int)ksizes2[i], keys2+koffset);
        koffset += ksizes2[i];
    }

    // ------------------------------------------------------

    char values2[64];

    /* getting multiple key/value pairs at once */
    ret = yk_get_packed(db_handle, YOKAN_MODE_DEFAULT, 4,
                       (const void*)keys2, ksizes2,
                       64, (void*)values2, vsizes2);
    assert(ret == YOKAN_SUCCESS);
    printf("Getting the values for the following keys:\n");
    koffset = 0;
    voffset = 0;
    for(int i=0; i < 4; i++) {
        if(vsizes2[i] == YOKAN_KEY_NOT_FOUND)
            printf("\t%.*s => (not found)\n", (int)ksizes2[i], keys2+koffset);
        else if(vsizes2[i] == YOKAN_SIZE_TOO_SMALL)
            printf("\t%.*s => (buffer too small)\n", (int)ksizes2[i], keys2+koffset);
        else {
            printf("\t%.*s => %.*s\n", (int)ksizes2[i], keys2+koffset,
                                   (int)vsizes2[i], values2+voffset);
            voffset += vsizes2[i];
        }
        koffset += ksizes2[i];
    }

    // ------------------------------------------------------

    char keys3[64];
    size_t ksizes3[4];

    ret = yk_list_keys_packed(db_handle, YOKAN_MODE_INCLUSIVE,
            "shane", 5, "", 0, 4, (void*)keys3, 64, ksizes3);
    assert(ret == YOKAN_SUCCESS);
    printf("Listed the following keys:\n");
    koffset = 0;
    for(int i=0; i < 4; i++) {
        if(ksizes3[i] == YOKAN_NO_MORE_KEYS)
            break;
        else if(ksizes3[i] == YOKAN_SIZE_TOO_SMALL)
            printf("\t(buffer too small)\n");
        else {
            printf("\t%.*s\n", (int)ksizes3[i], keys3+koffset);
            koffset += ksizes3[i];
        }
    }

    // ------------------------------------------------------

    char values3[64];
    size_t vsizes3[4];

    ret = yk_list_keyvals_packed(
            db_handle, YOKAN_MODE_INCLUSIVE,
            "shane", 5, "", 0, 4,
            (void*)keys3, 64, ksizes3,
            (void*)values3, 64, vsizes3);

    assert(ret == YOKAN_SUCCESS);
    printf("Listed the following key/value pairs:\n");
    koffset = 0;
    voffset = 0;
    for(int i=0; i < 4; i++) {
        if(ksizes3[i] == YOKAN_NO_MORE_KEYS)
            break;
        printf("\t%.*s => %.*s\n", (int)ksizes3[i], keys3+koffset,
                                   (int)vsizes3[i], values3+voffset);
        koffset += ksizes3[i];
        voffset += vsizes3[i];
    }

    // ------------------------------------------------------

    printf("Erasing all the key/value pairs.\n");
    ret = yk_erase_packed(db_handle, YOKAN_MODE_DEFAULT, 7,
                         (const void*)keys, ksizes);
    assert(ret == YOKAN_SUCCESS);

    // ------------------------------------------------------

    ret = yk_database_handle_release(db_handle);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_client_finalize(client);
    assert(ret == YOKAN_SUCCESS);

    margo_finalize(mid);

    return 0;
}
