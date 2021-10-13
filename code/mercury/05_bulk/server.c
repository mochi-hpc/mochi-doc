#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>
#include "types.h"

/* This structure will encapsulate data about the server. */
typedef struct {
    hg_class_t*     hg_class;
    hg_context_t*   hg_context;
} server_state;

typedef struct {
    char*       filename;
    hg_size_t   size;
    void*       buffer;
    hg_bulk_t   bulk_handle;
    hg_handle_t handle;
} rpc_state;

static hg_return_t save_bulk_completed(const struct hg_cb_info *info);
static hg_return_t save(hg_handle_t h);

int main(int argc, char** argv)
{
    hg_return_t ret;

    if(argc != 2) {
        printf("Usage: %s <server address>\n", argv[0]);
        exit(0);
    }

    const char* server_address = argv[1];

    server_state state; // Instance of the server's state

    state.hg_class = HG_Init(server_address, HG_TRUE);
    assert(state.hg_class != NULL);

    /* Get the address of the server */
    char hostname[128];
    hg_size_t hostname_size;
    hg_addr_t self_addr;
    HG_Addr_self(state.hg_class,&self_addr);
    HG_Addr_to_string(state.hg_class, hostname, &hostname_size, self_addr);
    printf("Server running at address %s\n",hostname);

    state.hg_context = HG_Context_create(state.hg_class);
    assert(state.hg_context != NULL);

    hg_id_t rpc_id = MERCURY_REGISTER(state.hg_class, "save", save_in_t, save_out_t, save);

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
    } while(1);

    ret = HG_Context_destroy(state.hg_context);
    assert(ret == HG_SUCCESS);

    ret = HG_Finalize(state.hg_class);
    assert(ret == HG_SUCCESS);

    return 0;
}

hg_return_t save(hg_handle_t handle)
{
    hg_return_t ret;
    save_in_t in;

    // Get the server_state attached to the RPC.
    const struct hg_info* info = HG_Get_info(handle);
    server_state* stt = HG_Registered_data(info->hg_class, info->id);

    ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    rpc_state* my_rpc_state = (rpc_state*)calloc(1,sizeof(rpc_state));
    my_rpc_state->handle = handle;
    my_rpc_state->filename = strdup(in.filename);
    my_rpc_state->size = in.size;
    my_rpc_state->buffer = calloc(1,in.size);

    ret = HG_Bulk_create(stt->hg_class, 1, &(my_rpc_state->buffer),
            &(my_rpc_state->size), HG_BULK_WRITE_ONLY, &(my_rpc_state->bulk_handle));
    assert(ret == HG_SUCCESS);

    /* initiate bulk transfer from client to server */
    ret = HG_Bulk_transfer(stt->hg_context, save_bulk_completed,
            my_rpc_state, HG_BULK_PULL, info->addr, in.bulk_handle, 0,
            my_rpc_state->bulk_handle, 0, my_rpc_state->size, HG_OP_ID_IGNORE);
    assert(ret == HG_SUCCESS);

    ret = HG_Free_input(handle, &in);
    assert(ret == HG_SUCCESS);
    return HG_SUCCESS;
}

hg_return_t save_bulk_completed(const struct hg_cb_info *info)
{
    assert(info->ret == 0);

    rpc_state* my_rpc_state = info->arg;
    hg_return_t ret;

    FILE* f = fopen(my_rpc_state->filename,"w+");
    fwrite(my_rpc_state->buffer, 1, my_rpc_state->size, f);
    fclose(f);

    printf("Writing file %s\n", my_rpc_state->filename);

    save_out_t out;
    out.ret = 0;

    ret = HG_Respond(my_rpc_state->handle, NULL, NULL, &out);
    assert(ret == HG_SUCCESS);
    (void)ret;

    HG_Bulk_free(my_rpc_state->bulk_handle);
    HG_Destroy(my_rpc_state->handle);
    free(my_rpc_state->filename);
    free(my_rpc_state->buffer);
    free(my_rpc_state);

    return HG_SUCCESS;
}
