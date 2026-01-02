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

        // Define provider description
        std::string provider_desc = R"(
        {
            "name": "my_dynamic_provider",
            "type": "yokan",
            "provider_id": 100,
            "pool": "__primary__",
            "config": {
                "database": {
                    "type": "map"
                }
            },
            "dependencies": {}
        }
        )";

        std::cout << "Adding provider..." << std::endl;

        uint16_t provider_id;
        service.addProvider(provider_desc, &provider_id);

        std::cout << "Provider added with ID: " << provider_id << std::endl;

        // Verify by querying configuration
        std::string config_str;
        service.getConfig(&config_str);
        auto config = nlohmann::json::parse(config_str);
        auto& providers = config["providers"];
        std::cout << "\nCurrent providers:" << std::endl;
        for(auto& provider : providers) {
            std::cout << "  - " << provider["name"]
                      << " (type=" << provider["type"]
                      << ", id=" << provider["provider_id"] << ")" << std::endl;
        }

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
