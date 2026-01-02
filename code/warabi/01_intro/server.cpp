/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <warabi/Provider.hpp>
#include <iostream>

namespace tl = thallium;

int main() {
    // Initialize Thallium engine
    tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

    // Configuration for memory backend
    auto config = R"(
    {
        "targets": [
            {
                "type": "memory",
                "config": {}
            }
        ]
    }
    )";

    // Create Warabi provider
    warabi::Provider provider(engine, 42, config);

    std::cout << "Warabi provider running at " << engine.self() << std::endl;

    engine.wait_for_finalize();

    return 0;
}
