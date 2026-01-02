/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <bedrock/AsyncRequest.hpp>
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

        std::cout << "=== Runtime Configuration Example ===" << std::endl;

        // 1. Query initial configuration
        std::cout << "\n1. Querying initial configuration..." << std::endl;
        std::string config_str;
        service.getConfig(&config_str);
        auto config = nlohmann::json::parse(config_str);
        size_t initial_providers = config["providers"].size();
        std::cout << "   Initial providers: " << initial_providers << std::endl;

        // 2. Add a new pool
        std::cout << "\n2. Adding new Argobots pool..." << std::endl;
        std::string pool_config = R"(
        {
            "name": "runtime_pool",
            "kind": "fifo_wait",
            "access": "mpmc"
        }
        )";
        service.addPool(pool_config);
        std::cout << "   Pool 'runtime_pool' added" << std::endl;

        // 3. Add a new execution stream
        std::cout << "\n3. Adding new execution stream..." << std::endl;
        std::string xstream_config = R"(
        {
            "name": "runtime_xstream",
            "scheduler": {
                "type": "basic_wait",
                "pools": ["runtime_pool"]
            }
        }
        )";
        service.addXstream(xstream_config);
        std::cout << "   Execution stream 'runtime_xstream' added" << std::endl;

        // 4. Add a provider using the new pool
        std::cout << "\n4. Adding provider with new pool..." << std::endl;
        std::string provider_desc = R"(
        {
            "name": "runtime_provider",
            "type": "yokan",
            "provider_id": 200,
            "pool": "runtime_pool",
            "config": {
                "database": {"type": "map"}
            }
        }
        )";
        uint16_t provider_id;
        bedrock::AsyncRequest request;
        service.addProvider(provider_desc, &provider_id, &request);
        request.wait();
        std::cout << "   Provider added with ID: " << provider_id << std::endl;

        // 5. Query updated configuration
        std::cout << "\n5. Querying updated configuration..." << std::endl;
        service.getConfig(&config_str);
        config = nlohmann::json::parse(config_str);
        size_t final_providers = config["providers"].size();
        std::cout << "   Final providers: " << final_providers << std::endl;

        // 6. Use Jx9 to list all pools
        std::cout << "\n6. Listing all pools via Jx9..." << std::endl;
        std::string jx9_script = R"(
            $result = [];
            if(array_key_exists('margo', $__config__) &&
               array_key_exists('argobots', $__config__['margo']) &&
               array_key_exists('pools', $__config__['margo']['argobots'])) {
                foreach($__config__['margo']['argobots']['pools'] as $pool) {
                    $result[] = $pool['name'];
                }
            }
            return $result;
        )";
        std::string result;
        service.queryConfig(jx9_script, &result);
        auto pools = nlohmann::json::parse(result);
        std::cout << "   Pools:" << std::endl;
        for(auto& pool : pools) {
            std::cout << "     - " << pool << std::endl;
        }

        std::cout << "\n=== Runtime configuration operations completed ===" << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
