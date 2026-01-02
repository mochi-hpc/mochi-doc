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

int main(int argc, char** argv)
{
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server> <provider_id>" << std::endl;
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

        std::cout << "Connected to Warabi target" << std::endl;

        // Prepare large data for transfer
        size_t data_size = 1024 * 1024;  // 1 MB
        std::vector<char> write_data(data_size, 'X');

        // Create a region large enough for the data
        warabi::RegionID region_id;
        target.create(&region_id, data_size);
        std::cout << "Created region: " << regionid_to_string(region_id) << std::endl;

        // Write large data
        std::cout << "Writing " << data_size << " bytes..." << std::endl;
        target.write(region_id, 0, write_data.data(), data_size);
        std::cout << "Write completed" << std::endl;

        // Read data back
        std::vector<char> read_data(data_size);
        std::cout << "Reading " << data_size << " bytes..." << std::endl;
        target.read(region_id, 0, read_data.data(), data_size);
        std::cout << "Read completed" << std::endl;

        // Verify data
        if(read_data == write_data) {
            std::cout << "SUCCESS: Data transfer verified!" << std::endl;
        } else {
            std::cout << "ERROR: Data mismatch!" << std::endl;
        }

        // Clean up
        target.erase(region_id);
        std::cout << "Region destroyed" << std::endl;

        std::cout << "\nLarge data transfer completed successfully!" << std::endl;
        std::cout << "Warabi efficiently handles bulk transfers using RDMA when available." << std::endl;

    } catch(const warabi::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
