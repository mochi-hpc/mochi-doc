#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>

static hg_class_t*     hg_class   = NULL; /* Pointer to the Mercury class */
static hg_context_t*   hg_context = NULL; /* Pointer to the Mercury context */
static hg_id_t         hello_rpc_id;      /* ID of the RPC */
static int completed = 0;                 /* Variable indicating if the call has completed */

/*
 * This callback will be called after looking up for the server's address.
 * This is the function that will also send the RPC to the servers, then
 * set the completed variable to 1.
 */
hg_return_t lookup_callback(const struct hg_cb_info *callback_info);

int main(int argc, char** argv)
{
    hg_return_t ret;

    if(argc != 3) {
        printf("Usage: %s <protocol> <server_address>\n",argv[0]);
        printf("Example: %s tcp ofi+tcp://1.2.3.4:1234\n",argv[0]);
        exit(0);
    }

    char* protocol = argv[1];
    char* server_address = argv[2];

    hg_class = HG_Init(protocol, HG_FALSE);
    assert(hg_class != NULL);

    hg_context = HG_Context_create(hg_class);
    assert(hg_context != NULL);

    /* Register a RPC function.
     * The first two NULL correspond to what would be pointers to
     * serialization/deserialization functions for input and output datatypes
     * (not used in this example).
     * The third NULL is the pointer to the function (which is on the server,
     * so NULL here on the client).
     */
    hello_rpc_id = HG_Register_name(hg_class, "hello", NULL, NULL, NULL);

    /* Indicate Mercury that we shouldn't expect a response from the server
     * when calling this RPC.
     */
    HG_Registered_disable_response(hg_class, hello_rpc_id, HG_TRUE);

    /* Lookup the address of the server, this is asynchronous and
     * the result will be handled by lookup_callback once we start the progress loop.
     * NULL correspond to a pointer to user data to pass to lookup_callback (we don't use
     * any here). The 4th argument is the address of the server.
     * The 5th argument is a pointer a variable of type hg_op_id_t, which identifies the operation.
     * It can be useful to get this identifier if we want to be able to cancel it using
     * HG_Cancel. Here we don't use it so we pass HG_OP_ID_IGNORE.
     */
    ret = HG_Addr_lookup(hg_context, lookup_callback, NULL, server_address, HG_OP_ID_IGNORE);

    /* Main event loop: we do some progress until completed becomes TRUE. */
    while(!completed)
    {
        unsigned int count;
        do {
            ret = HG_Trigger(hg_context, 0, 1, &count);
        } while((ret == HG_SUCCESS) && count && !completed);
        HG_Progress(hg_context, 100);
    }

    ret = HG_Context_destroy(hg_context);
    assert(ret == HG_SUCCESS);

    /* Finalize the hg_class. */
    hg_return_t err = HG_Finalize(hg_class);
    assert(err == HG_SUCCESS);
    return 0;
}

/*
 * This function is called when the address lookup operation has completed.
 */
hg_return_t lookup_callback(const struct hg_cb_info *callback_info)
{
    hg_return_t ret;

    /* First, check that the lookup went fine. */
    assert(callback_info->ret == 0);

    /* Get the address of the server. */
    hg_addr_t addr = callback_info->info.lookup.addr;

    /* Create a call to the hello_world RPC. */
    hg_handle_t handle;
    ret = HG_Create(hg_context, addr, hello_rpc_id, &handle);
    assert(ret == HG_SUCCESS);

    /* Send the RPC. The first NULL correspond to the callback
     * function to call when receiving the response from the server
     * (we don't expect a response, hence NULL here).
     * The second NULL is a pointer to user-specified data that will
     * be passed to the response callback.
     * The third NULL is a pointer to the RPC's argument (we don't
     * use any here).
     */
    ret = HG_Forward(handle, NULL, NULL, NULL);
    assert(ret == HG_SUCCESS);

    /* Free the handle */
    ret = HG_Destroy(handle);
    assert(ret == HG_SUCCESS);

    /* Set completed to 1 so we terminate the loop. */
    completed = 1;
    return HG_SUCCESS;
}
