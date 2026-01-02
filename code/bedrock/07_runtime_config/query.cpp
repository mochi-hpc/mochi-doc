/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <nlohmann/json.hpp>
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

        // Get the complete configuration
        std::string config_str;
        service.getConfig(&config_str);
        nlohmann::json config = nlohmann::json::parse(config_str);

        std::cout << "=== Complete Configuration ===" << std::endl;
        std::cout << config.dump(2) << std::endl;

        // Extract specific information
        std::cout << "\n=== Providers ===" << std::endl;
        if(config.contains("providers")) {
            for(auto& provider : config["providers"]) {
                std::cout << "  - " << provider["name"]
                          << " (type=" << provider["type"]
                          << ", id=" << provider["provider_id"] << ")" << std::endl;
            }
        }

        // Extract pool information
        std::cout << "\n=== Argobots Pools ===" << std::endl;
        if(config.contains("margo") &&
           config["margo"].contains("argobots") &&
           config["margo"]["argobots"].contains("pools")) {
            for(auto& pool : config["margo"]["argobots"]["pools"]) {
                std::cout << "  - " << pool["name"]
                          << " (kind=" << pool["kind"] << ")" << std::endl;
            }
        }

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
