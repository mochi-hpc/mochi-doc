#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>

typedef struct {
    hg_class_t*   hg_class;
    hg_context_t* hg_context;
    int           max_rpcs;
    int           num_rpcs;
} server_data_t;

hg_return_t hello_world(hg_handle_t h);

int main(int argc, char** argv)
{
    hg_return_t ret;

    if(argc != 2) {
        printf("Usage: %s <protocol>\n", argv[0]);
        exit(0);
    }

    server_data_t server_data = {
        .hg_class = NULL,
        .hg_context = NULL,
        .max_rpcs = 4,
        .num_rpcs = 0
    };

    server_data.hg_class = HG_Init(argv[1], HG_TRUE);
    assert(server_data.hg_class != NULL);

    char hostname[128];
    hg_size_t hostname_size;
    hg_addr_t self_addr;
    HG_Addr_self(server_data.hg_class, &self_addr);
    HG_Addr_to_string(server_data.hg_class, hostname, &hostname_size, self_addr);
    printf("Server running at address %s\n",hostname);
    HG_Addr_free(server_data.hg_class, self_addr);

    server_data.hg_context = HG_Context_create(server_data.hg_class);
    assert(server_data.hg_context != NULL);

    hg_id_t rpc_id = HG_Register_name(server_data.hg_class, "hello", NULL, NULL, hello_world);

    /* Register data with the RPC handler */
    HG_Register_data(server_data.hg_class, rpc_id, &server_data, NULL);

    HG_Registered_disable_response(server_data.hg_class, rpc_id, HG_TRUE);

    do
    {
        unsigned int count;
        do {
            ret = HG_Trigger(server_data.hg_context, 0, 1, &count);
        } while((ret == HG_SUCCESS) && count);
        HG_Progress(server_data.hg_context, 100);
    } while(server_data.num_rpcs < server_data.max_rpcs);

    ret = HG_Context_destroy(server_data.hg_context);
    assert(ret == HG_SUCCESS);

    ret = HG_Finalize(server_data.hg_class);
    assert(ret == HG_SUCCESS);

    return 0;
}

/* Implementation of the hello_world RPC. */
hg_return_t hello_world(hg_handle_t h)
{
    hg_return_t ret;

    /* Get the hg_class_t instance from the handle */
    const struct hg_info *info = HG_Get_info(h);
    hg_class_t* hg_class = info->hg_class;
    hg_id_t     rpc_id   = info->id;

    /* Get the data attached to the RPC handle */
    server_data_t* server_data = (server_data_t*)HG_Registered_data(hg_class, rpc_id);

    printf("Hello World!\n");
    server_data->num_rpcs += 1;

    ret = HG_Destroy(h);
    assert(ret == HG_SUCCESS);
    return HG_SUCCESS;
}
