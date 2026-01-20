/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <server_address> <provider_name> <new_pool>" << std::endl;
        return 1;
    }

    const char* server_addr = argv[1];
    const char* provider_name = argv[2];
    const char* new_pool = argv[3];

    try {
        thallium::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        bedrock::Client client(engine);
        bedrock::ServiceHandle service = client.makeServiceHandle(server_addr, 0);

        std::cout << "Changing pool for provider '" << provider_name
                  << "' to '" << new_pool << "'" << std::endl;

        // Change the provider's pool
        service.changeProviderPool(provider_name, new_pool);

        std::cout << "Pool changed successfully" << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
