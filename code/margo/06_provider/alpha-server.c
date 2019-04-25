#include "alpha-server.h"
#include "types.h"

struct alpha_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void alpha_finalize_provider_cb(void* p);

DECLARE_MARGO_RPC_HANDLER(alpha_sum_ult);
static void alpha_sum_ult(hg_handle_t h);
/* add other RPC declarations here */

int alpha_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        alpha_provider_t* provider)
{
    alpha_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "alpha_provider_register(): margo instance is not a server.");
        return ALPHA_FAILURE;
    }

    margo_provider_registered_name(mid, "alpha_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "alpha_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return ALPHA_FAILURE;
    }

    p = (alpha_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return ALPHA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_sum",
            sum_in_t, sum_out_t,
            alpha_sum_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    if(provider == ALPHA_PROVIDER_IGNORE)
        margo_push_finalize_callback(mid, &alpha_finalize_provider_cb, p);

    *provider = p;
    return ALPHA_SUCCESS;
}

int alpha_provider_destroy(
        alpha_provider_t provider)
{
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids */

    free(provider);
}

static void alpha_finalize_provider_cb(void* p)
{
    alpha_provider_t provider = (alpha_provider_t)p;
    alpha_provider_destroy(provider);
}

static void alpha_sum_ult(hg_handle_t h)
{
    hg_return_t ret;
    sum_in_t     in;
    sum_out_t   out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    out.ret = in.x + in.y;
    printf("Computed %d + %d = %d\n",in.x,in.y,out.ret);

    ret = margo_respond(h, &out);

    ret = margo_free_input(h, &in);
}
DEFINE_MARGO_RPC_HANDLER(alpha_sum_ult)
