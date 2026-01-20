/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <warabi/Client.hpp>
#include "../warabi_common.hpp"
#include <iostream>

namespace tl = thallium;

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server address> <provider id>\n";
        return -1;
    }

    const char* server_addr_str = argv[1];
    uint16_t provider_id = std::atoi(argv[2]);

    // Initialize Thallium in client mode
    tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

    try {
        // Create Warabi client
        warabi::Client client(engine);

        // Create target handle
        warabi::TargetHandle target = client.makeTargetHandle(
            server_addr_str, provider_id
        );

        // Create a region with 1KB size
        warabi::RegionID region_id;
        target.create(&region_id, 1024);

        std::cout << "Created region with ID: " << regionid_to_string(region_id) << std::endl;

        // Write data to the region
        std::string data = "Hello, Warabi!";
        target.write(region_id, 0, data.data(), data.size());
        std::cout << "Wrote " << data.size() << " bytes\n";

        // Read data back
        std::vector<char> buffer(data.size());
        target.read(region_id, 0, buffer.data(), buffer.size());

        std::string result(buffer.begin(), buffer.end());
        std::cout << "Read: " << result << std::endl;

        // Clean up
        target.erase(region_id);

    } catch(const warabi::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
