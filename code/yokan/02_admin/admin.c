#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <yokan/admin.h>

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
    yk_admin_t admin = YOKAN_ADMIN_NULL;
    yk_database_id_t db_id;

    ret = yk_admin_init(mid, &admin);
    assert(ret == YOKAN_SUCCESS);

    ret = yk_open_database(admin, server_addr, provider_id,
            "ABCD", "map", "{}", &db_id);
    assert(ret == YOKAN_SUCCESS);

    yk_database_id_t db_ids[4];
    size_t db_count = 4;

    ret = yk_list_databases(admin, server_addr, provider_id,
            "ABCD", db_ids, &db_count);
    assert(ret == YOKAN_SUCCESS);

    printf("Found %lu databases in provider:\n", db_count);
    for(int i=0; i < db_count; i++) {
        char db_id_str[37];
        yk_database_id_to_string(db_ids[i], db_id_str);
        printf("\t%s\n", db_id_str);
    }

    ret = yk_close_database(admin, server_addr, provider_id,
            "ABCD", db_ids[0]);
    assert(ret == yokan_success);

    ret = yk_destroy_database(admin, server_addr, provider_id,
            "ABCD", db_ids[1]);
    assert(ret == yokan_success);

    ret = yk_admin_finalize(admin);
    assert(ret == yokan_success);

    margo_finalize(mid);

    return 0;
}
