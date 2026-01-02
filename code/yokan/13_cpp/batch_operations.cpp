/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <iostream>
#include <vector>
#include <string>

namespace tl = thallium;

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_addr> <provider_id>" << std::endl;
        return 1;
    }

    try {
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create a client using the Margo instance from Thallium
        yokan::Client client(engine.get_margo_instance());

        // Look up the server address
        tl::endpoint server_ep = engine.lookup(argv[1]);

        // Create database handle
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(), std::atoi(argv[2]));

        // Prepare multiple key/value pairs
        std::vector<std::string> keys = {"user:1", "user:2", "user:3"};
        std::vector<std::string> values = {"Alice", "Bob", "Carol"};

        // Convert to raw pointers for batch API
        std::vector<const void*> key_ptrs;
        std::vector<size_t> key_sizes;
        std::vector<const void*> value_ptrs;
        std::vector<size_t> value_sizes;

        for(const auto& k : keys) {
            key_ptrs.push_back(k.data());
            key_sizes.push_back(k.size());
        }
        for(const auto& v : values) {
            value_ptrs.push_back(v.data());
            value_sizes.push_back(v.size());
        }

        // Put multiple key/value pairs at once
        db.putMulti(keys.size(),
                    key_ptrs.data(), key_sizes.data(),
                    value_ptrs.data(), value_sizes.data());
        std::cout << "Stored " << keys.size() << " key/value pairs" << std::endl;

        // Get multiple values at once
        std::vector<std::vector<char>> buffers(keys.size(), std::vector<char>(256));
        std::vector<void*> buffer_ptrs;
        std::vector<size_t> buffer_sizes;

        for(auto& buf : buffers) {
            buffer_ptrs.push_back(buf.data());
            buffer_sizes.push_back(buf.size());
        }

        db.getMulti(keys.size(),
                    key_ptrs.data(), key_sizes.data(),
                    buffer_ptrs.data(), buffer_sizes.data());

        std::cout << "Retrieved values:" << std::endl;
        for(size_t i = 0; i < keys.size(); i++) {
            std::string value(buffers[i].data(), buffer_sizes[i]);
            std::cout << "  " << keys[i] << " = " << value << std::endl;
        }

        // Check existence of multiple keys
        std::vector<bool> existence = db.existsMulti(keys.size(),
                                                      key_ptrs.data(), key_sizes.data());

        std::cout << "Existence check:" << std::endl;
        for(size_t i = 0; i < keys.size(); i++) {
            std::cout << "  " << keys[i] << ": " << (existence[i] ? "exists" : "missing") << std::endl;
        }

        // Erase multiple keys
        db.eraseMulti(keys.size(), key_ptrs.data(), key_sizes.data());
        std::cout << "Erased " << keys.size() << " keys" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
