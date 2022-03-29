#include <assert.h>
#include <stdio.h>
#include <ssg.h>

int main(int argc, char** argv)
{
    margo_instance_id mid = margo_init("tcp", MARGO_SERVER_MODE, 1, 0);
    assert(mid);

    int ret = ssg_init();
    assert(ret == SSG_SUCCESS);

    ret = ssg_finalize();
    assert(ret == SSG_SUCCESS);

    margo_finalize(mid);

    return 0;
}
