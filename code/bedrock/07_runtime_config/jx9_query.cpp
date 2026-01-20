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

        // Jx9 script to extract provider names
        std::string jx9_script = R"(
            $result = [];
            foreach($__config__['providers'] as $provider) {
                $result[] = $provider['name'];
            }
            return $result;
        )";

        std::cout << "Querying provider names with Jx9..." << std::endl;

        // Execute the Jx9 query
        std::string result;
        service.queryConfig(jx9_script, &result);

        // Parse the result
        auto provider_names = nlohmann::json::parse(result);

        std::cout << "Provider names:" << std::endl;
        for(auto& name : provider_names) {
            std::cout << "  - " << name << std::endl;
        }

        // Another Jx9 query to count providers by type
        std::string count_script = R"(
            $counts = [];
            foreach($__config__['providers'] as $provider) {
                $type = $provider['type'];
                if(!array_key_exists($type, $counts)) {
                    $counts[$type] = 0;
                }
                $counts[$type] += 1;
            }
            return $counts;
        )";

        service.queryConfig(count_script, &result);
        auto provider_counts = nlohmann::json::parse(result);

        std::cout << "\nProviders by type:" << std::endl;
        for(auto& [type, count] : provider_counts.items()) {
            std::cout << "  - " << type << ": " << count << std::endl;
        }

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
