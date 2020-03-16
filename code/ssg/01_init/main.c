#include <assert.h>
#include <stdio.h>
#include <ssg.h>

int main(int argc, char** argv)
{
    int ret = ssg_init();
    assert(ret == SSG_SUCCESS);

    ret = ssg_finalize();
    assert(ret == SSG_SUCCESS);

    return 0;
}
