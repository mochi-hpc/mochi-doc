#include <assert.h>
#include <stdio.h>
#include <margo.h>

int main(int argc, char** argv)
{
    struct margo_init_info args = {
        .json_config   = NULL,
        .progress_pool = NULL,
        .rpc_pool      = NULL,
        .hg_class      = NULL,
        .hg_context    = NULL,
        .hg_init_info  = NULL
    };

    margo_instance_id mid = margo_init_ext("tcp", MARGO_SERVER_MODE, &args);
    assert(mid);

    char* config = margo_get_config(mid);
    printf("%s\n", config);
    free(config);

    margo_finalize(mid);

    return 0;
}
