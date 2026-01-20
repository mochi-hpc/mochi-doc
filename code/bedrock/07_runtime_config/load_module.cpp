/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_address> <module_path>" << std::endl;
        return 1;
    }

    const char* server_addr = argv[1];
    const char* module_path = argv[2];

    try {
        thallium::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        bedrock::Client client(engine);
        bedrock::ServiceHandle service = client.makeServiceHandle(server_addr, 0);

        std::cout << "Loading module: " << module_path << std::endl;

        // Load the module
        service.loadModule(module_path);

        std::cout << "Module loaded successfully" << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
