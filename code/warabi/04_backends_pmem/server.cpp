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

    // Configure Warabi with persistent memory backend
    // PMEM backend: uses persistent memory for data storage
    std::string config = R"(
    {
        "target": {
            "type": "pmem",
            "config": {
                "path": "/tmp/warabi.pmem",
                "create_if_missing_with_size": 10737418240
            }
        ]
    }
    )";

    // Create Warabi provider with pmem backend
    uint16_t provider_id = 42;
    warabi::Provider provider(engine, provider_id, config);

    std::cout << "Warabi provider running with PMEM backend" << std::endl;
    std::cout << "Server address: " << engine.self() << std::endl;
    std::cout << "Provider ID: " << provider_id << std::endl;
    std::cout << "\nPMEM backend configuration:" << std::endl;
    std::cout << "  - Path: /tmp/warabi.pmem" << std::endl;
    std::cout << "  - Size: 10 GB" << std::endl;
    std::cout << "\nPMEM backend characteristics:" << std::endl;
    std::cout << "  - Data stored in persistent memory" << std::endl;
    std::cout << "  - Very fast performance (near DRAM speed)" << std::endl;
    std::cout << "  - Full persistence across restarts" << std::endl;

    engine.wait_for_finalize();
    return 0;
}
