#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mercury.h>

static hg_class_t*     hg_class     = NULL; /* Pointer to the Mercury class */
static hg_context_t*   hg_context   = NULL; /* Pointer to the Mercury context */

int main(int argc, char** argv)
{
    hg_return_t ret;
    /*
     * Initialize an hg_class.
     * Here we only specify the protocal since this is a client
     * (no need for an address and a port). HG_FALSE indicates that
     * the client will not listen for incoming requests.
     */
    hg_class = HG_Init("tcp", HG_FALSE);
    assert(hg_class != NULL);

    /* Creates a context for the hg_class. */
    hg_context = HG_Context_create(hg_class);
    assert(hg_context != NULL);

    /* Destroy the context. */
    ret = HG_Context_destroy(hg_context);
    assert(ret == HG_SUCCESS);

    /* Finalize the hg_class. */
    hg_return_t err = HG_Finalize(hg_class);
    assert(err == HG_SUCCESS);
    return 0;
}

