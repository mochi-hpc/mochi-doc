/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <remi/remi-server.h>
#include <remi/remi-client.h>
#include <warabi/Provider.hpp>
#include <warabi/Client.hpp>
#include <iostream>
#include <vector>
#include <string>

int main() {
    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);

    // Set up REMI
    remi_client_t remi_client;
    remi_client_init(engine.get_margo_instance(),
                     ABT_IO_INSTANCE_NULL, &remi_client);

    remi_provider_t remi_provider;
    remi_provider_register(engine.get_margo_instance(),
                           ABT_IO_INSTANCE_NULL, 3,
                           ABT_POOL_NULL, &remi_provider);

    // Create source provider with memory backend
    std::string source_config = R"({
        "target": {
            "type": "memory",
            "config": {}
        }
    })";
    warabi::Provider provider1(engine, 1, source_config,
                                engine.get_handler_pool(),
                                remi_client, REMI_PROVIDER_NULL);

    // Create destination provider with empty config
    warabi::Provider provider2(engine, 2, "{}",
                                engine.get_handler_pool(),
                                REMI_CLIENT_NULL, remi_provider);

    // Create client and target handle
    warabi::Client client(engine);
    std::string addr = engine.self();
    auto target1 = client.makeTargetHandle(addr, 1);

    // Create some regions in the source
    std::vector<warabi::RegionID> regions;
    for(int i = 0; i < 10; i++) {
        warabi::RegionID rid;
        std::string data = "Data for region " + std::to_string(i);
        target1.createAndWrite(&rid, data.data(), data.size(), true);
        regions.push_back(rid);
    }

    std::cout << "Created 10 regions in source provider\n";

    // Migrate target from provider 1 to provider 2
    std::string migration_options = R"({
        "new_root": "/tmp/warabi-migrated",
        "transfer_size": 1024,
        "merge_config": {},
        "remove_source": true
    })";

    std::cout << "Migrating target...\n";
    provider1.migrateTarget(addr, 2, migration_options);
    std::cout << "Migration complete!\n";

    // Now access the regions from the destination provider
    auto target2 = client.makeTargetHandle(addr, 2);

    // Verify all regions are accessible from destination
    for(const auto& rid : regions) {
        char buffer[100];
        target2.read(rid, 0, buffer, sizeof(buffer));
        std::cout << "Read from migrated region: " << buffer << "\n";
    }

    // Source provider is now invalid
    try {
        warabi::RegionID rid;
        target1.createAndWrite(&rid, "test", 4);
    } catch(const warabi::Exception& ex) {
        std::cout << "Source provider correctly invalid after migration\n";
    }

    remi_client_finalize(remi_client);
    return 0;
}
