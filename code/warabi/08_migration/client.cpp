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
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <source_server> <source_provider_id>"
                  << " <dest_server> <dest_provider_id>" << std::endl;
        return -1;
    }

    try {
        // Initialize client
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        warabi::Client client(engine);

        // Get source and destination target handles
        warabi::TargetHandle source = client.makeTargetHandle(
            argv[1], std::atoi(argv[2])
        );

        warabi::TargetHandle dest = client.makeTargetHandle(
            argv[3], std::atoi(argv[4])
        );

        std::cout << "=== Warabi Data Migration ===" << std::endl;

        // Create region on source target
        warabi::RegionID source_region;
        source.create(&source_region, 1024);
        std::cout << "Created source region: " << regionid_to_string(source_region) << std::endl;

        // Write data to source
        std::string message = "Data to be migrated across targets";
        source.write(source_region, 0, message.data(), message.size());
        std::cout << "Wrote " << message.size() << " bytes to source" << std::endl;

        // Migrate region manually: read from source, write to destination
        std::cout << "\nMigrating region..." << std::endl;

        // Read data from source region
        std::vector<char> buffer(message.size());
        source.read(source_region, 0, buffer.data(), message.size());
        std::cout << "Read data from source region" << std::endl;

        // Create destination region and write data
        warabi::RegionID dest_region;
        dest.create(&dest_region, message.size());
        dest.write(dest_region, 0, buffer.data(), buffer.size());
        std::cout << "Wrote data to destination region" << std::endl;

        std::cout << "Migration completed!" << std::endl;
        std::cout << "  Source region: " << regionid_to_string(source_region) << std::endl;
        std::cout << "  Destination region: " << regionid_to_string(dest_region) << std::endl;

        // Verify data on destination
        std::vector<char> verify_buffer(message.size());
        dest.read(dest_region, 0, verify_buffer.data(), message.size());
        std::string result(verify_buffer.begin(), verify_buffer.end());

        std::cout << "\nVerification:" << std::endl;
        std::cout << "  Original: " << message << std::endl;
        std::cout << "  Migrated: " << result << std::endl;

        if(result == message) {
            std::cout << "SUCCESS: Migration verified!" << std::endl;
        } else {
            std::cout << "ERROR: Data mismatch after migration!" << std::endl;
        }

        // Clean up
        source.erase(source_region);
        dest.erase(dest_region);
        std::cout << "\nRegions destroyed" << std::endl;

    } catch(const warabi::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
