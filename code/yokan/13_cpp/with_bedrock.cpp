/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <bedrock/Client.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <iostream>
#include <string>

namespace tl = thallium;

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bedrock_config.json>" << std::endl;
        return 1;
    }

    try {
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create Bedrock client
        bedrock::Client bedrock_client(engine);

        // Load Bedrock configuration
        std::string config_file = argv[1];

        std::cout << "Loading Bedrock configuration from: " << config_file << std::endl;

        // Query Bedrock for Yokan provider information
        // (In a real application, you would parse the config or use Bedrock API
        //  to discover providers. This is a simplified example.)

        // For this example, assume we know the server address and provider ID
        std::string server_addr = "na+sm://12345-0";
        uint16_t provider_id = 1;

        // Create Yokan client
        yokan::Client yokan_client(engine.get_margo_instance());

        // Look up server
        tl::endpoint server_ep = engine.lookup(server_addr);

        // Create database handle
        yokan::Database db = yokan_client.makeDatabaseHandle(
            server_ep.get_addr(), provider_id);

        std::cout << "Connected to Yokan provider via Bedrock" << std::endl;

        // Use Yokan database
        std::string key = "bedrock:test";
        std::string value = "Yokan + Bedrock integration";

        db.put(key.data(), key.size(), value.data(), value.size());
        std::cout << "Stored: " << key << " = " << value << std::endl;

        // Retrieve value
        std::vector<char> buffer(256);
        size_t vsize = buffer.size();
        db.get(key.data(), key.size(), buffer.data(), &vsize);

        std::string retrieved(buffer.data(), vsize);
        std::cout << "Retrieved: " << retrieved << std::endl;

        // Example Bedrock configuration (for reference):
        std::cout << "\nExample Bedrock configuration snippet:" << std::endl;
        std::cout << R"({
    "libraries": {
        "yokan": "libyokan-bedrock-module.so"
    },
    "providers": [
        {
            "name": "my_yokan_db",
            "type": "yokan",
            "provider_id": 1,
            "config": {
                "database": {
                    "type": "map"
                }
            }
        }
    ]
})" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Yokan error: " << ex.what() << std::endl;
        return 1;
    } catch(const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
