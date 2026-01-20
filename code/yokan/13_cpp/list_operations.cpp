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

        // Insert test data
        std::vector<std::string> keys = {"user:001", "user:002", "user:003", "user:004"};
        std::vector<std::string> values = {"Alice", "Bob", "Carol", "Dave"};

        for(size_t i = 0; i < keys.size(); i++) {
            db.put(keys[i].data(), keys[i].size(),
                   values[i].data(), values[i].size());
        }

        std::cout << "Inserted " << keys.size() << " records" << std::endl;

        // List key-value pairs using iter
        std::string from_key = "user:";
        std::string filter = "";

        std::cout << "\nListing key-value pairs:" << std::endl;

        // Create callback using C++ lambda
        auto keyval_callback = [](size_t index, const void* key, size_t ksize,
                                   const void* val, size_t vsize) -> yk_return_t {
            std::string k(static_cast<const char*>(key), ksize);
            std::string v(static_cast<const char*>(val), vsize);
            std::cout << "  [" << index << "] " << k << " = " << v << std::endl;
            return YOKAN_SUCCESS;
        };

        db.iter(from_key.data(), from_key.size(),
                filter.data(), filter.size(),
                0, /* count = 0 means all */
                keyval_callback);

        std::cout << "\nIteration completed successfully!" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
