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

    // Configure Warabi with memory backend
    // Memory backend: stores data in RAM, data is lost on shutdown
    auto config = R"(
    {
        "target": {
            "type": "memory",
            "config": {}
        }
    }
    )";

    // Create Warabi provider with memory backend
    uint16_t provider_id = 42;
    warabi::Provider provider(engine, provider_id, config);

    std::cout << "Warabi provider running with MEMORY backend" << std::endl;
    std::cout << "Server address: " << engine.self() << std::endl;
    std::cout << "Provider ID: " << provider_id << std::endl;
    std::cout << "\nMemory backend characteristics:" << std::endl;
    std::cout << "  - Data stored in RAM" << std::endl;
    std::cout << "  - Fastest performance" << std::endl;
    std::cout << "  - No persistence (data lost on shutdown)" << std::endl;

    engine.wait_for_finalize();
    return 0;
}
