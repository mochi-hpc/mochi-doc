#include <assert.h>
#include <stdio.h>
#include <ssg.h>
#include <stdlib.h>

static void my_membership_update_cb(void* uargs,
        ssg_member_id_t member_id,
        ssg_member_update_type_t update_type)
{
    switch(update_type) {
    case SSG_MEMBER_JOINED:
        printf("Member %ld joined\n", member_id);
        break;
    case SSG_MEMBER_LEFT:
        printf("Member %ld left\n", member_id);
        break;
    case SSG_MEMBER_DIED:
        printf("Member %ld died\n", member_id);
        break;
    }
}

int main(int argc, char** argv)
{
    int ret = ssg_init();
    assert(ret == SSG_SUCCESS);

    margo_instance_id mid = margo_init("tcp", MARGO_SERVER_MODE, 1, 0);
    assert(mid);

    hg_addr_t my_addr;
    margo_addr_self(mid, &my_addr);
    char my_addr_str[128];
    size_t my_addr_str_size = 128;
    margo_addr_to_string(mid, my_addr_str, &my_addr_str_size, my_addr);
    margo_addr_free(mid, my_addr);

    const char* group_addr_strs[] = { my_addr_str };
    ssg_group_config_t config = {
        .swim_period_length_ms = 1000,
        .swim_suspect_timeout_periods = 5,
        .swim_subgroup_member_count = -1,
        .ssg_credential = -1
    };

    ssg_group_id_t gid;
    ret = ssg_group_create(
            mid, "mygroup", group_addr_strs, 1,
            &config, my_membership_update_cb, NULL, &gid);

    // get the current process' member id
    ssg_member_id_t self_id;
    ret = ssg_get_self_id(mid, &self_id);
    // get the group size
    int size;
    ret = ssg_get_group_size(gid, &size);
    // get the address from a member id (here self_id)
    hg_addr_t self_addr;
    ret = ssg_get_group_member_addr(gid, self_id, &self_addr);
    // get the rank of the current process in the group
    int self_rank;
    ret = ssg_get_group_self_rank(gid, &self_rank);
    // get the rank from a member id (here self_id)
    int rank;
    ret = ssg_get_group_member_rank(gid, self_id, &rank);
    // get a member id from a rank
    ssg_member_id_t member_id;
    ret = ssg_get_group_member_id_from_rank(gid, rank, &member_id);
    // get an array of member ids from a range of ranks [0,size[
    ssg_member_id_t member_ids[size];
    ret = ssg_get_group_member_ids_from_range(gid, 0, size, member_ids);
    // get a string address from a rank
    char* addr_str;
    ret = ssg_get_group_member_addr_str(gid, 0, &addr_str);
    free(addr_str);

    ret = ssg_group_leave(gid);
    assert(ret == SSG_SUCCESS);

    margo_finalize(mid);

    ret = ssg_finalize();
    assert(ret == SSG_SUCCESS);

    return 0;
}
