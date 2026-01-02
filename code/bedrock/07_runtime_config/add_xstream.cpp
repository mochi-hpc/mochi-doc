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
        thallium::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        bedrock::Client client(engine);
        bedrock::ServiceHandle service = client.makeServiceHandle(argv[1], 0);

        // First, add a pool for the xstream to use
        std::string pool_config = R"(
        {
            "name": "xstream_pool",
            "kind": "fifo_wait",
            "access": "mpmc"
        }
        )";
        service.addPool(pool_config);
        std::cout << "Created pool for execution stream" << std::endl;

        // Define execution stream configuration
        std::string xstream_config = R"(
        {
            "name": "my_dynamic_xstream",
            "cpubind": 1,
            "scheduler": {
                "type": "basic_wait",
                "pools": ["xstream_pool"]
            }
        }
        )";

        std::cout << "Adding execution stream..." << std::endl;

        // Add the execution stream
        service.addXstream(xstream_config);

        std::cout << "Execution stream added successfully" << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
