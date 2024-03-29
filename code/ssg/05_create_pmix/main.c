#include <mpi.h>
#include <assert.h>
#include <stdio.h>
#include <ssg.h>
#include <ssg-pmix.h>

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
    pmix_proc_t proc;
    PMIx_Init(&proc, NULL, 0);

    margo_instance_id mid = margo_init("tcp", MARGO_SERVER_MODE, 1, 0);
    assert(mid);

    int ret = ssg_init();
    assert(ret == SSG_SUCCESS);

    ssg_group_config_t config = {
        .swim_period_length_ms = 1000,
        .swim_suspect_timeout_periods = 5,
        .swim_subgroup_member_count = -1,
        .swim_disabled = 0,
        .ssg_credential = -1
    };

    ssg_group_id_t gid;
    ret = ssg_group_create_pmix(
            mid, "mygroup", proc,
            &config, my_membership_update_cb, NULL, &gid);

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

    PMIx_Finalize(NULL, 0);

    return 0;
}
