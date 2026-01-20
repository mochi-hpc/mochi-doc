/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <warabi/Client.hpp>
#include "../warabi_common.hpp"
#include <warabi/AsyncRequest.hpp>
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

        std::cout << "=== Warabi Asynchronous Operations ===" << std::endl;

        // Create a region
        warabi::RegionID region_id;
        target.create(&region_id, 1024);
        std::cout << "Created region: " << regionid_to_string(region_id) << std::endl;

        // Prepare data for async write
        std::string message = "Asynchronous write test data";
        std::vector<char> write_buffer(message.begin(), message.end());

        // Issue asynchronous write
        std::cout << "\nIssuing async write..." << std::endl;
        warabi::AsyncRequest write_req;
        target.write(region_id, 0, write_buffer.data(), write_buffer.size(), &write_req);
        std::cout << "Async write issued (non-blocking)" << std::endl;

        // Do other work while write is in progress
        std::cout << "Performing other work while write completes..." << std::endl;

        // Wait for write to complete
        write_req.wait();
        std::cout << "Async write completed!" << std::endl;

        // Issue asynchronous read
        std::cout << "\nIssuing async read..." << std::endl;
        std::vector<char> read_buffer(write_buffer.size());

        warabi::AsyncRequest read_req;
        target.read(region_id, 0, read_buffer.data(), write_buffer.size(), &read_req);
        std::cout << "Async read issued (non-blocking)" << std::endl;

        // Do other work while read is in progress
        std::cout << "Performing other work while read completes..." << std::endl;

        // Wait for read to complete
        read_req.wait();
        std::cout << "Async read completed!" << std::endl;

        // Verify data
        std::string result(read_buffer.begin(), read_buffer.end());
        std::cout << "\nVerification:" << std::endl;
        std::cout << "  Original: " << message << std::endl;
        std::cout << "  Read:     " << result << std::endl;

        if(result == message) {
            std::cout << "SUCCESS: Data verified!" << std::endl;
        } else {
            std::cout << "ERROR: Data mismatch!" << std::endl;
        }

        // Test multiple concurrent async operations
        std::cout << "\n=== Testing concurrent async operations ===" << std::endl;

        // Create multiple regions
        std::vector<warabi::RegionID> regions;
        for(int i = 0; i < 3; i++) {
            warabi::RegionID rid;
            target.create(&rid, 1024);
            regions.push_back(rid);
        }

        // Issue multiple async writes concurrently
        std::vector<warabi::AsyncRequest> requests;
        for(size_t i = 0; i < regions.size(); i++) {
            std::string data = "Data for region " + std::to_string(i);
            std::vector<char> buf(data.begin(), data.end());

            warabi::AsyncRequest req;
            target.write(regions[i], 0, buf.data(), buf.size(), &req);
            requests.push_back(std::move(req));
            std::cout << "Issued async write " << i << std::endl;
        }

        // Wait for all writes
        std::cout << "Waiting for all async writes..." << std::endl;
        for(auto& req : requests) {
            req.wait();
        }
        std::cout << "All async writes completed!" << std::endl;

        // Clean up
        target.erase(region_id);
        for(auto& rid : regions) {
            target.erase(rid);
        }
        std::cout << "\nAsync operations completed successfully!" << std::endl;

    } catch(const warabi::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
