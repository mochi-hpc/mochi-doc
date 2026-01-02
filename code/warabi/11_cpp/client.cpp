/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <warabi/Client.hpp>
#include "../warabi_common.hpp"
#include <warabi/TargetHandle.hpp>
#include <iostream>
#include <vector>
#include <string>

namespace tl = thallium;

int main(int argc, char** argv)
{
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server> <provider_id>" << std::endl;
        return -1;
    }

    try {
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        std::cout << "=== Warabi C++ API Client ===" << std::endl;

        // Initialize Warabi client
        warabi::Client client(engine);
        std::cout << "Client initialized" << std::endl;

        // Create target handle
        warabi::TargetHandle target = client.makeTargetHandle(
            argv[1], std::atoi(argv[2])
        );
        std::cout << "Connected to target" << std::endl;

        // Create region
        warabi::RegionID region_id;
        target.create(&region_id, 1024);
        std::cout << "Created region: " << regionid_to_string(region_id) << std::endl;

        // Write using C++ containers
        std::string message = "Hello from C++ API!";
        std::vector<char> write_data(message.begin(), message.end());

        target.write(region_id, 0, write_data.data(), write_data.size());
        std::cout << "Wrote: " << message << std::endl;

        // Read using C++ containers (read the same amount we wrote)
        std::vector<char> read_data(write_data.size());

        target.read(region_id, 0, read_data.data(), write_data.size());
        std::string result(read_data.begin(), read_data.end());
        std::cout << "Read: " << result << std::endl;

        // Verify
        if(result == message) {
            std::cout << "SUCCESS: Data verified!" << std::endl;
        } else {
            std::cout << "ERROR: Data mismatch!" << std::endl;
        }

        // Clean up
        target.erase(region_id);
        std::cout << "Region destroyed" << std::endl;

        std::cout << "\nC++ API demonstration completed!" << std::endl;

    } catch(const warabi::Exception& ex) {
        // C++ exception handling
        std::cerr << "Warabi exception: " << ex.what() << std::endl;
        return -1;
    } catch(const std::exception& ex) {
        std::cerr << "Standard exception: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
