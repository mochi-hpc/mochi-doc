/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace tl = thallium;

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_addr> <provider_id>" << std::endl;
        return 1;
    }

    try {
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        yokan::Client client(engine.get_margo_instance());
        tl::endpoint server_ep = engine.lookup(argv[1]);
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(), std::atoi(argv[2]));

        // Working with std::string directly
        std::string key = "user:1001";
        std::string value = "Alice Johnson";

        // Put using string helper
        db.put(key.data(), key.size(), value.data(), value.size());

        // Get with automatic sizing using std::vector
        std::vector<char> result;

        // First get the size
        size_t vsize = db.length(key.data(), key.size());

        // Resize vector and get data
        result.resize(vsize);
        db.get(key.data(), key.size(), result.data(), &vsize);

        // Convert to string
        std::string retrieved(result.data(), vsize);
        std::cout << "Retrieved: " << retrieved << std::endl;

        // Working with multiple keys using vectors
        std::vector<std::string> keys = {"user:1001", "user:1002", "user:1003"};
        std::vector<std::string> values = {"Alice", "Bob", "Carol"};

        // Convert to pointer/size arrays for batch operations
        std::vector<const void*> key_ptrs;
        std::vector<size_t> key_sizes;
        std::vector<const void*> val_ptrs;
        std::vector<size_t> val_sizes;

        for(const auto& k : keys) {
            key_ptrs.push_back(k.data());
            key_sizes.push_back(k.size());
        }

        for(const auto& v : values) {
            val_ptrs.push_back(v.data());
            val_sizes.push_back(v.size());
        }

        // Batch put
        db.putMulti(keys.size(), key_ptrs.data(), key_sizes.data(),
                    val_ptrs.data(), val_sizes.data());

        std::cout << "Stored " << keys.size() << " key-value pairs" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
