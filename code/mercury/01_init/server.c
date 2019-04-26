#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>

static hg_class_t*     hg_class   = NULL; /* the mercury class */
static hg_context_t*   hg_context = NULL; /* the mercury context */

int main(int argc, char** argv)
{
    hg_return_t ret;

    /* Initialize Mercury and get an hg_class handle.
     * bmi+tcp is the protocol to use.
     * localhost is the address of the server (not useful at the server itself).
     * HG_TRUE is here to specify that mercury will listen for incoming requests.
     * (HG_TRUE on servers, HG_FALSE on clients).
     */
    hg_class = HG_Init("tcp", HG_TRUE);
    assert(hg_class != NULL);

    /* Get the address of the server */
    char hostname[128];
    hg_size_t hostname_size;
    hg_addr_t self_addr;
    HG_Addr_self(hg_class, &self_addr);
    HG_Addr_to_string(hg_class, hostname, &hostname_size, self_addr);
    printf("Server running at address %s\n",hostname);
    HG_Addr_free(hg_class, self_addr);

    /* Creates a Mercury context from the Mercury class. */
    hg_context = HG_Context_create(hg_class);
    assert(hg_context != NULL);

    /* Progress loop */
    do
    {
        /* count will keep track of how many RPCs were treated in a given
         * call to HG_Trigger.
         */
        unsigned int count;
        do {
            /* Executes callbacks.
             * 0 = no timeout, the function just returns if there is nothing to process.
             * 1 = the max number of callbacks to execute before returning.
             * After the call, count will hold the number of callbacks executed.
             */
            ret = HG_Trigger(hg_context, 0, 1, &count);
        } while((ret == HG_SUCCESS) && count);
        /* Exit the loop if no event has been processed. */

        /* Make progress on receiving/sending data.
         * 100 is the timeout in milliseconds, for which to wait for network events. */
        HG_Progress(hg_context, 100);
    } while(1); /* another condition should be put here for the loop to terminate */

    /* Destroys the Mercury context. */
    ret = HG_Context_destroy(hg_context);
    assert(ret == HG_SUCCESS);

    /* Finalize Mercury. */
    ret = HG_Finalize(hg_class);
    assert(ret == HG_SUCCESS);

    return 0;
}
