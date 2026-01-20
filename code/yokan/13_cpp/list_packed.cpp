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
        for(int i = 1; i <= 5; i++) {
            std::string key = "item:" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            db.put(key.data(), key.size(), value.data(), value.size());
        }

        // List keys using packed format
        std::string from_key = "item:";
        std::string filter = "";

        // Allocate buffer for packed keys
        std::vector<char> packed_keys(1024);
        std::vector<size_t> key_sizes(10);
        size_t count = 10;

        db.listKeysPacked(from_key.data(), from_key.size(),
                          filter.data(), filter.size(),
                          count,
                          packed_keys.data(), packed_keys.size(),
                          key_sizes.data());

        std::cout << "Listed " << count << " keys:" << std::endl;

        // Parse packed keys
        size_t offset = 0;
        for(size_t i = 0; i < count; i++) {
            std::string key(packed_keys.data() + offset, key_sizes[i]);
            std::cout << "  " << key << std::endl;
            offset += key_sizes[i];
        }

        // List key-value pairs using packed format
        std::vector<char> packed_vals(1024);
        std::vector<size_t> val_sizes(10);
        count = 10;

        db.listKeyValsPacked(from_key.data(), from_key.size(),
                             filter.data(), filter.size(),
                             count,
                             packed_keys.data(), packed_keys.size(),
                             key_sizes.data(),
                             packed_vals.data(), packed_vals.size(),
                             val_sizes.data());

        std::cout << "\nListed " << count << " key-value pairs:" << std::endl;

        // Parse packed key-value pairs
        size_t key_offset = 0;
        size_t val_offset = 0;
        for(size_t i = 0; i < count; i++) {
            std::string key(packed_keys.data() + key_offset, key_sizes[i]);
            std::string value(packed_vals.data() + val_offset, val_sizes[i]);
            std::cout << "  " << key << " = " << value << std::endl;
            key_offset += key_sizes[i];
            val_offset += val_sizes[i];
        }

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
