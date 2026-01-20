/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <flock/cxx/server.hpp>
#include <iostream>

namespace tl = thallium;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

    std::cout << "Server running at address: " << engine.self() << std::endl;

    // Initialize group view with self as the only member
    flock::GroupView initial_view;
    uint16_t provider_id = 42;

    // Add self to the group view
    std::string my_address = static_cast<std::string>(engine.self());
    initial_view.members().add(my_address.c_str(), provider_id);

    // Provider configuration (static backend)
    std::string config = R"(
    {
        "group": {
            "type": "static",
            "config": {}
        }
    }
    )";

    // Create Flock provider
    flock::Provider provider(engine, provider_id, config.c_str(), initial_view);

    std::cout << "Flock provider registered with provider_id="
              << provider_id << std::endl;

    // Wait for finalize
    engine.wait_for_finalize();

    return 0;
}
