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

        std::cout << "=== Warabi Region Management ===" << std::endl;

        // Create multiple regions
        std::vector<warabi::RegionID> regions;
        for(int i = 0; i < 3; i++) {
            warabi::RegionID region_id;
            target.create(&region_id, 1024);
            regions.push_back(region_id);
            std::cout << "Created region " << i << ": " << regionid_to_string(region_id) << std::endl;
        }

        // Write different data to each region and track sizes
        std::vector<size_t> data_sizes;
        for(size_t i = 0; i < regions.size(); i++) {
            std::string message = "Data for region " + std::to_string(i);
            target.write(regions[i], 0, message.data(), message.size());
            data_sizes.push_back(message.size());
            std::cout << "Wrote to region " << i << ": " << message << std::endl;
        }

        // Show sizes of data written
        std::cout << "\nData sizes written:" << std::endl;
        for(size_t i = 0; i < regions.size(); i++) {
            std::cout << "  Region " << i << ": " << data_sizes[i] << " bytes" << std::endl;
        }

        // Read back from regions
        std::cout << "\nReading from regions:" << std::endl;
        for(size_t i = 0; i < regions.size(); i++) {
            std::vector<char> buffer(data_sizes[i]);
            target.read(regions[i], 0, buffer.data(), data_sizes[i]);
            std::string data(buffer.begin(), buffer.end());
            std::cout << "  Region " << i << ": " << data << std::endl;
        }

        // Persist a region (flush to storage)
        std::cout << "\nPersisting region 0..." << std::endl;
        target.persist(regions[0], 0, data_sizes[0]);
        std::cout << "Region 0 persisted" << std::endl;

        // Destroy regions
        std::cout << "\nDestroying regions..." << std::endl;
        for(size_t i = 0; i < regions.size(); i++) {
            target.erase(regions[i]);
            std::cout << "  Destroyed region " << i << std::endl;
        }

        std::cout << "\nRegion operations completed successfully!" << std::endl;

    } catch(const warabi::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
