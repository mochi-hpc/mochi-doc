/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <warabi/Provider.hpp>
#include <iostream>

namespace tl = thallium;

int main()
{
    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

    std::cout << "=== Warabi C++ API Server ===" << std::endl;
    std::cout << "Server address: " << engine.self() << std::endl;

    // Configure Warabi with multiple backends
    auto config = R"(
    {
        "targets": [
            {
                "type": "memory",
                "config": {}
            },
            {
                "type": "abt-io",
                "config": {
                    "path": "/tmp/warabi",
                    "num_threads": 2
                }
            }
        ]
    }
    )";

    // Create Warabi provider
    uint16_t provider_id = 42;
    warabi::Provider provider(engine, provider_id, config);

    std::cout << "Warabi provider registered (C++ API)" << std::endl;
    std::cout << "Provider ID: " << provider_id << std::endl;
    std::cout << "Configured targets:" << std::endl;
    std::cout << "  - Target 0: memory backend" << std::endl;
    std::cout << "  - Target 1: abt-io backend (/tmp/warabi)" << std::endl;

    std::cout << "\nC++ API benefits:" << std::endl;
    std::cout << "  - Exception-based error handling" << std::endl;
    std::cout << "  - RAII resource management" << std::endl;
    std::cout << "  - Type-safe interfaces" << std::endl;
    std::cout << "  - STL integration" << std::endl;

    engine.wait_for_finalize();
    return 0;
}
