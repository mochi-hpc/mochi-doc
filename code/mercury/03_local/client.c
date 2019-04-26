#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>

typedef struct {
    hg_class_t*   hg_class;
    hg_context_t* hg_context;
    hg_id_t       hello_rpc_id;
    int           completed;
} client_data_t;

hg_return_t lookup_callback(const struct hg_cb_info *callback_info);

int main(int argc, char** argv)
{
    hg_return_t ret;

    if(argc != 3) {
        printf("Usage: %s <protocol> <server_address>\n",argv[0]);
        printf("Example: %s tcp ofi+tcp://1.2.3.4:1234\n",argv[0]);
        exit(0);
    }

    client_data_t client_data = {
        .hg_class     = NULL,
        .hg_context   = NULL,
        .hello_rpc_id = 0,
        .completed    = 0
    };

    char* protocol = argv[1];
    char* server_address = argv[2];

    client_data.hg_class = HG_Init(protocol, HG_FALSE);
    assert(client_data.hg_class != NULL);

    client_data.hg_context = HG_Context_create(client_data.hg_class);
    assert(client_data.hg_context != NULL);

    client_data.hello_rpc_id = HG_Register_name(client_data.hg_class, "hello", NULL, NULL, NULL);

    HG_Registered_disable_response(client_data.hg_class, client_data.hello_rpc_id, HG_TRUE);

    /* We pass a pointer to the client's data as 3rd argument */
    ret = HG_Addr_lookup(client_data.hg_context, lookup_callback, &client_data, server_address, HG_OP_ID_IGNORE);

    while(!client_data.completed)
    {
        unsigned int count;
        do {
            ret = HG_Trigger(client_data.hg_context, 0, 1, &count);
        } while((ret == HG_SUCCESS) && count && !client_data.completed);
        HG_Progress(client_data.hg_context, 100);
    }

    ret = HG_Context_destroy(client_data.hg_context);
    assert(ret == HG_SUCCESS);

    hg_return_t err = HG_Finalize(client_data.hg_class);
    assert(err == HG_SUCCESS);
    return 0;
}

hg_return_t lookup_callback(const struct hg_cb_info *callback_info)
{
    hg_return_t ret;
    assert(callback_info->ret == 0);

    /* Get the client's data */
    client_data_t* client_data = (client_data_t*)(callback_info->arg);

    hg_addr_t addr = callback_info->info.lookup.addr;

    hg_handle_t handle;
    ret = HG_Create(client_data->hg_context, addr, client_data->hello_rpc_id, &handle);
    assert(ret == HG_SUCCESS);

    ret = HG_Forward(handle, NULL, NULL, NULL);
    assert(ret == HG_SUCCESS);

    ret = HG_Destroy(handle);
    assert(ret == HG_SUCCESS);

    client_data->completed = 1;
    return HG_SUCCESS;
}
