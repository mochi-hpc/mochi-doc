#include <assert.h>
#include <stdio.h>
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

    ssg_group_id_t gid;

    int num_addrs = 1;
    ret = ssg_group_id_load("mygroup.ssg", &num_addrs, &gid);
    assert(ret == SSG_SUCCESS);

    ret = ssg_group_join(mid, gid, my_membership_update_cb, NULL);
    assert(ret == SSG_SUCCESS);

    margo_thread_sleep(mid, 10000);
    // ...
    // do stuff using the group
    // ...

    ret = ssg_group_leave(gid);
    assert(ret == SSG_SUCCESS);

    ret = ssg_group_destroy(gid);
    assert(ret == SSG_SUCCESS);

    ret = ssg_finalize();
    assert(ret == SSG_SUCCESS);

    margo_finalize(mid);

    return 0;
}
