/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <server_address> <provider_name> <snapshot_path>"
                  << std::endl;
        return 1;
    }

    const char* server_addr = argv[1];
    const char* provider_name = argv[2];
    const char* snapshot_path = argv[3];

    try {
        thallium::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        bedrock::Client client(engine);
        bedrock::ServiceHandle service = client.makeServiceHandle(server_addr, 0);

        std::cout << "Snapshotting provider '" << provider_name << "'" << std::endl;
        std::cout << "  Destination: " << snapshot_path << std::endl;

        // Snapshot configuration (provider-specific)
        std::string snapshot_config = "{}";

        // Create snapshot
        // remove_source=false means keep the provider running after snapshot
        service.snapshotProvider(
            provider_name,
            snapshot_path,
            snapshot_config,
            false  // remove_source
        );

        std::cout << "Snapshot created successfully" << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
