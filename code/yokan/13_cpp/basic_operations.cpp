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
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create a client using the Margo instance from Thallium
        yokan::Client client(engine.get_margo_instance());

        // Look up the server address
        tl::endpoint server_ep = engine.lookup(argv[1]);

        // Create database handle
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(), std::atoi(argv[2]));

        // Put operation
        std::string key = "user:1001";
        std::string value = "Alice Johnson";
        db.put(key.data(), key.size(), value.data(), value.size());
        std::cout << "Stored: " << key << " = " << value << std::endl;

        // Get operation
        std::vector<char> buffer(256);
        size_t vsize = buffer.size();
        db.get(key.data(), key.size(), buffer.data(), &vsize);
        std::string retrieved(buffer.data(), vsize);
        std::cout << "Retrieved: " << retrieved << std::endl;

        // Exists operation
        bool exists = db.exists(key.data(), key.size());
        std::cout << "Key exists: " << (exists ? "yes" : "no") << std::endl;

        // Length operation
        size_t length = db.length(key.data(), key.size());
        std::cout << "Value length: " << length << " bytes" << std::endl;

        // Erase operation
        db.erase(key.data(), key.size());
        std::cout << "Key erased" << std::endl;

        // Verify erasure
        bool exists_after = db.exists(key.data(), key.size());
        std::cout << "Key exists after erase: " << (exists_after ? "yes" : "no") << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
