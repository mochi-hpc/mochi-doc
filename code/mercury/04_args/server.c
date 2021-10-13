#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>
#include "types.h"

typedef struct {
    hg_class_t*     hg_class;
    hg_context_t*   hg_context;
    int             num_rpcs;
} server_state;

static const int TOTAL_RPCS = 10;

hg_return_t sum(hg_handle_t h);

int main(int argc, char** argv)
{
    hg_return_t ret;

    if(argc != 2) {
        printf("Usage: %s <server address>\n", argv[0]);
        exit(0);
    }

    const char* server_address = argv[1];

    server_state state; // Instance of the server's state
    state.num_rpcs = 0;

    state.hg_class = HG_Init(server_address, HG_TRUE);
    assert(state.hg_class != NULL);

    char hostname[128];
    hg_size_t hostname_size;
    hg_addr_t self_addr;
    HG_Addr_self(state.hg_class, &self_addr);
    HG_Addr_to_string(state.hg_class, hostname, &hostname_size, self_addr);
    printf("Server running at address %s\n",hostname);
    HG_Addr_free(state.hg_class, self_addr);

    state.hg_context = HG_Context_create(state.hg_class);
    assert(state.hg_context != NULL);

    hg_id_t rpc_id = MERCURY_REGISTER(state.hg_class, "sum", sum_in_t, sum_out_t, sum);

    /* Attach the local server_state to the RPC so we can get a pointer to it when
     * the RPC is invoked. */
    ret = HG_Register_data(state.hg_class, rpc_id, &state, NULL);

    do
    {
        unsigned int count;
        do {
            ret = HG_Trigger(state.hg_context, 0, 1, &count);
        } while((ret == HG_SUCCESS) && count);

        HG_Progress(state.hg_context, 100);
    } while(state.num_rpcs < TOTAL_RPCS);

    ret = HG_Context_destroy(state.hg_context);
    assert(ret == HG_SUCCESS);

    ret = HG_Finalize(state.hg_class);
    assert(ret == HG_SUCCESS);

    return 0;
}

hg_return_t sum(hg_handle_t handle)
{
    hg_return_t ret;
    sum_in_t in;
    sum_out_t out;

    const struct hg_info* info = HG_Get_info(handle);
    server_state* state = HG_Registered_data(info->hg_class, info->id);

    ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    out.ret = in.x + in.y;
    printf("%d + %d = %d\n",in.x,in.y,in.x+in.y);
    state->num_rpcs += 1;

    ret = HG_Respond(handle,NULL,NULL,&out);
    assert(ret == HG_SUCCESS);

    ret = HG_Free_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ret = HG_Destroy(handle);
    assert(ret == HG_SUCCESS);

    return HG_SUCCESS;
}
