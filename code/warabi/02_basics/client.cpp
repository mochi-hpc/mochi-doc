/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <warabi/Client.hpp>
#include "../warabi_common.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace tl = thallium;

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server> <provider_id>\n";
        return -1;
    }

    try {
        // Initialize client
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        warabi::Client client(engine);

        // Get target handle
        warabi::TargetHandle target = client.makeTargetHandle(
            argv[1], std::atoi(argv[2])
        );

        // Create a region
        warabi::RegionID region_id;
        target.create(&region_id, 1024);
        std::cout << "Created region: " << regionid_to_string(region_id) << std::endl;

        // Write data
        std::string message = "Hello, Warabi! This is a test message.";
        target.write(region_id, 0, message.data(), message.size());
        std::cout << "Wrote " << message.size() << " bytes\n";

        // Read data back (read the same amount we wrote)
        std::vector<char> buffer(message.size());
        target.read(region_id, 0, buffer.data(), message.size());

        std::string result(buffer.begin(), buffer.end());
        std::cout << "Read: " << result << std::endl;

        // Verify
        if(result == message) {
            std::cout << "SUCCESS: Data matches!\n";
        } else {
            std::cout << "ERROR: Data mismatch!\n";
        }

        // Clean up
        target.erase(region_id);
        std::cout << "Region destroyed\n";

    } catch(const warabi::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
