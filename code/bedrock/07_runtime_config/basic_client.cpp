/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server_address>" << std::endl;
        return 1;
    }

    try {
        // Initialize Thallium engine
        thallium::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create Bedrock client
        bedrock::Client client(engine);

        // Create service handle
        // Provider ID 0 is the default Bedrock provider
        bedrock::ServiceHandle service = client.makeServiceHandle(argv[1], 0);

        std::cout << "Connected to Bedrock service" << std::endl;

        // Get configuration
        std::string config;
        service.getConfig(&config);
        std::cout << "Current configuration:" << std::endl;
        std::cout << config << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Bedrock error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
