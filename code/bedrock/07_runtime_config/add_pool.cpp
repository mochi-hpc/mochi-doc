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

        // Define pool configuration as JSON
        std::string pool_config = R"(
        {
            "name": "my_dynamic_pool",
            "kind": "fifo_wait",
            "access": "mpmc"
        }
        )";

        std::cout << "Adding Argobots pool..." << std::endl;

        // Add the pool
        service.addPool(pool_config);

        std::cout << "Pool added successfully" << std::endl;

        // Verify by querying configuration
        std::string config_str;
        service.getConfig(&config_str);
        auto config = nlohmann::json::parse(config_str);
        auto& pools = config["margo"]["argobots"]["pools"];
        std::cout << "Current pools:" << std::endl;
        for(auto& pool : pools) {
            std::cout << "  - " << pool["name"] << std::endl;
        }

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
