/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <flock/Client.hpp>
#include <flock/GroupHandle.hpp>
#include <iostream>

namespace tl = thallium;

int main(int argc, char** argv)
{
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <server_address> <provider_id>" << std::endl;
        return -1;
    }

    std::string server_addr = argv[1];
    uint16_t provider_id = std::atoi(argv[2]);

    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

    // Initialize Flock client
    flock::Client client(engine);

    // Create group handle
    flock::GroupHandle group = client.makeGroupHandle(server_addr, provider_id);

    std::cout << "Connected to Flock group" << std::endl;

    // Get group view
    flock::GroupView view = group.getView();

    std::cout << "\n=== Group Information ===" << std::endl;
    std::cout << "Group size: " << view.members.size() << " members" << std::endl;
    std::cout << "View digest: " << view.digest << std::endl;

    // Print all members
    std::cout << "\nGroup members:" << std::endl;
    for(size_t i = 0; i < view.members.size(); i++) {
        const auto& member = view.members[i];
        std::cout << "  [" << i << "] Rank: " << member.rank << std::endl;
        std::cout << "      Address: " << member.address << std::endl;
        std::cout << "      Provider ID: " << member.provider_id << std::endl;
    }

    // Get number of members
    size_t num_members = group.getNumMembers();
    std::cout << "\nNumber of members: " << num_members << std::endl;

    std::cout << "\nFlock C++ client operations completed successfully" << std::endl;

    return 0;
}
