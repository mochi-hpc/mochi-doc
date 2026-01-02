/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <bedrock/AsyncRequest.hpp>
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

        // Create an async request object
        bedrock::AsyncRequest request;

        // Define provider configuration
        std::string provider_desc = R"(
        {
            "name": "async_provider",
            "type": "yokan",
            "provider_id": 101,
            "pool": "__primary__",
            "config": {
                "database": {"type": "map"}
            }
        }
        )";

        std::cout << "Issuing asynchronous addProvider request..." << std::endl;

        // Issue async request (non-blocking)
        uint16_t provider_id;
        service.addProvider(provider_desc, &provider_id, &request);

        std::cout << "Request issued, continuing other work..." << std::endl;

        // Do other work here while the request is being processed
        // ...

        std::cout << "Waiting for request to complete..." << std::endl;

        // Wait for the request to complete
        request.wait();

        std::cout << "Provider added asynchronously with ID: " << provider_id << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
