/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <flock/Provider.hpp>
#include <iostream>

namespace tl = thallium;

int main(int argc, char** argv)
{
    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

    std::cout << "Server running at address: " << engine.self() << std::endl;

    // Bootstrap configuration
    flock::BootstrapMethod bootstrap = flock::BootstrapMethod::Self();

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
    uint16_t provider_id = 42;
    flock::Provider provider(engine, provider_id, config, bootstrap);

    std::cout << "Flock provider registered with provider_id="
              << provider_id << std::endl;

    // Wait for finalize
    engine.wait_for_finalize();

    return 0;
}
