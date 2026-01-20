/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/Client.hpp>
#include <bedrock/ServiceHandle.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <source_address> <provider_name> <dest_address> <dest_provider_id>"
                  << std::endl;
        return 1;
    }

    const char* source_addr = argv[1];
    const char* provider_name = argv[2];
    const char* dest_addr = argv[3];
    uint16_t dest_provider_id = std::atoi(argv[4]);

    try {
        thallium::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        bedrock::Client client(engine);
        bedrock::ServiceHandle service = client.makeServiceHandle(source_addr, 0);

        std::cout << "Migrating provider '" << provider_name << "'" << std::endl;
        std::cout << "  From: " << source_addr << std::endl;
        std::cout << "  To: " << dest_addr << " (provider ID " << dest_provider_id << ")" << std::endl;

        // Migration configuration (provider-specific)
        std::string migration_config = "{}";

        // Migrate provider state
        // remove_source=false means keep the source after migration
        service.migrateProvider(
            provider_name,
            dest_addr,
            dest_provider_id,
            migration_config,
            false  // remove_source
        );

        std::cout << "Migration completed successfully" << std::endl;

    } catch(const bedrock::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
