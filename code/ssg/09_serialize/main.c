#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ssg.h>

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
    margo_instance_id mid = margo_init("tcp", MARGO_SERVER_MODE, 1, 0);
    assert(mid);

    int ret = ssg_init();
    assert(ret == SSG_SUCCESS);

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
        .swim_disabled = 0,
        .ssg_credential = -1
    };

    ssg_group_id_t gid;
    ret = ssg_group_create(
            mid, "mygroup", group_addr_strs, 1,
            &config, my_membership_update_cb, NULL, &gid);

    // serialization into a buffer
    char* buf = NULL;
    size_t buf_size = 0;
    ssg_group_id_serialize(gid, 1, &buf, &buf_size);

    // deserialization from a buffer
    ssg_group_id_t gid2;
    int num_addrs = 1;
    ssg_group_id_deserialize(buf, buf_size, &num_addrs, &gid2);

    // the buffer needs to be freed after serialization
    free(buf);

    ret = ssg_group_leave(gid);
    assert(ret == SSG_SUCCESS);

    ret = ssg_group_destroy(gid);
    assert(ret == SSG_SUCCESS);

    ret = ssg_finalize();
    assert(ret == SSG_SUCCESS);

    margo_finalize(mid);

    return 0;
}
