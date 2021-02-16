/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include <abt-io.h>
#include <string.h>

static struct bedrock_dependency ModuleA_dependencies[] = {
    { "io", "abt_io", BEDROCK_REQUIRED },
    { "sdskv_ph", "sdskv", BEDROCK_ARRAY | BEDROCK_REQUIRED },
    BEDROCK_NO_MORE_DEPENDENCIES
};

static int ModuleA_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid  = bedrock_args_get_margo_instance(args);
    uint16_t provider_id   = bedrock_args_get_provider_id(args);
    ABT_pool pool          = bedrock_args_get_pool(args);
    const char* config     = bedrock_args_get_config(args);
    const char* name       = bedrock_args_get_name(args);

    abt_io_instance_id* io = bedrock_args_get_dependency(args, "io", 0);
    size_t num_databases   = bedrock_args_get_num_dependencies(args, "sdskv_ph");
    int i;
    for(i=0; i < num_databases; i++) {
        void* sdskv_ph = bedrock_args_get_dependency(args, "sdskv_ph", i);
        /* ... */
    }

    *provider = strdup("ModuleA:provider"); // just to put something in *provider
    printf("Registered a provider from module A\n");
    printf(" -> mid         = %p\n", (void*)mid);
    printf(" -> provider id = %d\n", provider_id);
    printf(" -> pool        = %p\n", (void*)pool);
    printf(" -> config      = %s\n", config);
    printf(" -> name        = %s\n", name);
    return BEDROCK_SUCCESS;
}

static int ModuleA_deregister_provider(
        bedrock_module_provider_t provider)
{
    free(provider);
    printf("Deregistered a provider from module A\n");
    return BEDROCK_SUCCESS;
}

static char* ModuleA_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    return strdup("{}");
}

static int ModuleA_init_client(
        margo_instance_id mid,
        bedrock_module_client_t* client)
{
    *client = strdup("ModuleA:client");
    printf("Registered a client from module A\n");
    printf(" -> mid = %p\n", (void*)mid);
    return BEDROCK_SUCCESS;
}

static int ModuleA_finalize_client(
        bedrock_module_client_t client)
{
    free(client);
    printf("Finalized a client from module A\n");
    return BEDROCK_SUCCESS;
}

static int ModuleA_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    (void)client;
    (void)address;
    (void)provider_id;
    *ph = strdup("ModuleA:provider_handle");
    printf("Created provider handle from module A\n");
    return BEDROCK_SUCCESS;
}

static int ModuleA_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    free(ph);
    printf("Destroyed provider handle from module A\n");
    return BEDROCK_SUCCESS;
}

static struct bedrock_module ModuleA = {
    .register_provider       = ModuleA_register_provider,
    .deregister_provider     = ModuleA_deregister_provider,
    .get_provider_config     = ModuleA_get_provider_config,
    .init_client             = ModuleA_init_client,
    .finalize_client         = ModuleA_finalize_client,
    .create_provider_handle  = ModuleA_create_provider_handle,
    .destroy_provider_handle = ModuleA_destroy_provider_handle,
    .dependencies            = ModuleA_dependencies
};

BEDROCK_REGISTER_MODULE(module_a, ModuleA)
