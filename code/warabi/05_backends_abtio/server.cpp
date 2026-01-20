/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <warabi/Provider.hpp>
#include <iostream>

namespace tl = thallium;

int main(int argc, char** argv)
{
    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

    // Configure Warabi with ABT-IO backend
    // ABT-IO backend: uses Argobots-based asynchronous I/O for file storage
    std::string config = R"(
    {
        "target": {
            "type": "abtio",
            "config": {
                "path": "/tmp/warabi.dat",
                "create_if_missing": true
            }
        }
    }
    )";

    // Create Warabi provider with abt-io backend
    uint16_t provider_id = 42;
    warabi::Provider provider(engine, provider_id, config);

    std::cout << "Warabi provider running with ABT-IO backend" << std::endl;
    std::cout << "Server address: " << engine.self() << std::endl;
    std::cout << "Provider ID: " << provider_id << std::endl;
    std::cout << "\nABT-IO backend configuration:" << std::endl;
    std::cout << "  - Path: /tmp/warabi.dat" << std::endl;
    std::cout << "  - I/O threads: 4" << std::endl;
    std::cout << "\nABT-IO backend characteristics:" << std::endl;
    std::cout << "  - Data stored on regular filesystem" << std::endl;
    std::cout << "  - Asynchronous I/O using Argobots" << std::endl;
    std::cout << "  - Configurable thread pool for I/O operations" << std::endl;
    std::cout << "  - Good balance of performance and persistence" << std::endl;

    engine.wait_for_finalize();
    return 0;
}
