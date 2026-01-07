/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <flock/cxx/client.hpp>
#include <iostream>

namespace tl = thallium;

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <flock_address> <flock_provider_id>" << std::endl;
        return 1;
    }

    std::string flock_addr = argv[1];
    uint16_t flock_provider_id = std::atoi(argv[2]);

    try {
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create Flock client
        flock::Client client(engine);

        // Create group handle
        flock::GroupHandle group = client.makeGroupHandle(
            flock_addr, flock_provider_id
        );

        std::cout << "=== Service Discovery via Flock ===" << std::endl;

        // Get group view
        flock::GroupView view = group.view();

        std::cout << "Group size: " << view.members().count() << " members" << std::endl;
        std::cout << "\nService members:" << std::endl;

        // List all members
        for(size_t i = 0; i < view.members().count(); i++) {
            const auto& member = view.members()[i];
            std::cout << "  [" << i << "]" << std::endl;
            std::cout << "      Address: " << member.address << std::endl;
            std::cout << "      Provider ID: " << member.provider_id << std::endl;

            // You can now connect to services at these addresses
            // For example, to connect to a Yokan provider at this address:
            // yokan::Client yokan_client(engine);
            // auto yokan_db = yokan_client.makeDatabaseHandle(
            //     member.address, 42 /* yokan provider id */);
        }

        std::cout << "\n=== Service discovery completed ===" << std::endl;

    } catch(const flock::Exception& ex) {
        std::cerr << "Flock error: " << ex.what() << std::endl;
        return 1;
    } catch(const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
