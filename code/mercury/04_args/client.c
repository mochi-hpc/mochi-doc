#include <assert.h>
#include <stdio.h>
#include <mercury.h>
#include "types.h"

typedef struct {
    hg_class_t*   hg_class;
    hg_context_t* hg_context;
    hg_id_t       sum_rpc_id;
    int           completed;
} client_state_t;

hg_return_t lookup_callback(const struct hg_cb_info *callback_info);
hg_return_t sum_completed(const struct hg_cb_info *info);

int main(int argc, char** argv)
{
    hg_return_t ret;

    if(argc != 3) {
        printf("Usage: %s <protocol> <server_address>\n",argv[0]);
        printf("Example: %s tcp tcp://1.2.3.4:1234\n",argv[0]);
        exit(0);
    }
    char* protocol = argv[1];
    char* server_address = argv[2];

    client_state_t state;
    state.completed = 0;

    state.hg_class = HG_Init(protocol, HG_FALSE);
    assert(state.hg_class != NULL);

    state.hg_context = HG_Context_create(state.hg_class);
    assert(state.hg_context != NULL);

    state.sum_rpc_id = MERCURY_REGISTER(state.hg_class, "sum", sum_in_t, sum_out_t, NULL);

    ret = HG_Addr_lookup(state.hg_context, lookup_callback, &state, server_address, HG_OP_ID_IGNORE);

    while(!state.completed)
    {
        unsigned int count;
        do {
            ret = HG_Trigger(state.hg_context, 0, 1, &count);
        } while((ret == HG_SUCCESS) && count && !state.completed);
        HG_Progress(state.hg_context, 100);
    }

    ret = HG_Context_destroy(state.hg_context);
    assert(ret == HG_SUCCESS);

    hg_return_t err = HG_Finalize(state.hg_class);
    assert(err == HG_SUCCESS);
    return 0;
}


hg_return_t lookup_callback(const struct hg_cb_info *callback_info)
{
    hg_return_t ret;

    /* We get the pointer to the engine_state here. */
    client_state_t* state = (client_state_t*)(callback_info->arg);

    assert(callback_info->ret == 0);
    hg_addr_t addr = callback_info->info.lookup.addr;

    hg_handle_t handle;
    ret = HG_Create(state->hg_context, addr, state->sum_rpc_id, &handle);
    assert(ret == HG_SUCCESS);

    sum_in_t in;
    in.x = 42;
    in.y = 23;

    ret = HG_Forward(handle, sum_completed, state, &in);
    assert(ret == HG_SUCCESS);

    ret = HG_Addr_free(state->hg_class, addr);
    assert(ret == HG_SUCCESS);

    return HG_SUCCESS;
}

hg_return_t sum_completed(const struct hg_cb_info *info)
{
    hg_return_t ret;

    client_state_t* state = (client_state_t*)(info->arg);

    sum_out_t out;
    assert(info->ret == HG_SUCCESS);

    ret = HG_Get_output(info->info.forward.handle, &out);
    assert(ret == HG_SUCCESS);

    printf("Got response: %d\n", out.ret);

    ret = HG_Free_output(info->info.forward.handle, &out);
    assert(ret == HG_SUCCESS);

    ret = HG_Destroy(info->info.forward.handle);
    assert(ret == HG_SUCCESS);

    state->completed = 1;

    return HG_SUCCESS;
}
