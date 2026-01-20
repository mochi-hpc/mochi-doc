/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <flock/cxx/client.hpp>
#include <flock/cxx/group.hpp>
#include <iostream>
#include <cstdlib>

namespace tl = thallium;

int main(int argc, char** argv)
{
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <server_address> <provider_id>" << std::endl;
        return -1;
    }

    std::string server_addr_str = argv[1];
    uint16_t provider_id = std::atoi(argv[2]);

    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

    // Initialize Flock client
    flock::Client client(engine);

    // Lookup server address
    tl::endpoint server_endpoint = engine.lookup(server_addr_str);

    // Create group handle
    flock::GroupHandle group = client.makeGroupHandle(
        server_endpoint.get_addr(), provider_id, 0);

    std::cout << "Connected to Flock group" << std::endl;

    // Get group view
    flock::GroupView view = group.view();

    std::cout << "\n=== Group Information ===" << std::endl;
    std::cout << "Group size: " << view.members().count() << " members" << std::endl;
    std::cout << "View digest: " << view.digest() << std::endl;

    // Print all members
    std::cout << "\nGroup members:" << std::endl;
    auto members = view.members();
    for(size_t i = 0; i < members.count(); i++) {
        auto member = members[i];
        std::cout << "  [" << i << "] Address: " << member.address << std::endl;
        std::cout << "      Provider ID: " << member.provider_id << std::endl;
    }

    // Print metadata if any
    auto metadata = view.metadata();
    if(metadata.count() > 0) {
        std::cout << "\nMetadata:" << std::endl;
        for(size_t i = 0; i < metadata.count(); i++) {
            auto md = metadata[i];
            std::cout << "  " << md.key << " = " << md.value << std::endl;
        }
    }

    std::cout << "\nFlock C++ client operations completed successfully" << std::endl;

    return 0;
}
