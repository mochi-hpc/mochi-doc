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

        // Example 1: Handling missing keys
        std::cout << "=== Example 1: Missing key ===" << std::endl;
        try {
            std::string key = "nonexistent";
            std::vector<char> buffer(256);
            size_t size = buffer.size();
            db.get(key.data(), key.size(), buffer.data(), &size);
        } catch(const yokan::Exception& ex) {
            std::cout << "Expected error - key not found: " << ex.what() << std::endl;
        }

        // Example 2: Safe get with existence check
        std::cout << "\n=== Example 2: Safe get ===" << std::endl;
        std::string key = "safe_key";
        if(db.exists(key.data(), key.size())) {
            std::vector<char> buffer(256);
            size_t size = buffer.size();
            db.get(key.data(), key.size(), buffer.data(), &size);
            std::string value(buffer.data(), size);
            std::cout << "Value: " << value << std::endl;
        } else {
            std::cout << "Key doesn't exist, using default" << std::endl;
        }

        // Example 3: Exception information
        std::cout << "\n=== Example 3: Exception details ===" << std::endl;
        try {
            std::string missing_key = "another_missing_key";
            std::vector<char> buffer(256);
            size_t size = buffer.size();
            db.get(missing_key.data(), missing_key.size(), buffer.data(), &size);
        } catch(const yokan::Exception& ex) {
            std::cout << "Exception message: " << ex.what() << std::endl;
            // The exception message contains error details
        }

        // Example 4: RAII ensures cleanup even with exceptions
        std::cout << "\n=== Example 4: RAII cleanup ===" << std::endl;
        {
            yokan::Database scoped_db = client.makeDatabaseHandle(
                server_ep.get_addr(), std::atoi(argv[2]));

            try {
                // Even if this throws...
                std::string key = "will_fail";
                std::vector<char> buffer(256);
                size_t size = buffer.size();
                scoped_db.get(key.data(), key.size(), buffer.data(), &size);
            } catch(const yokan::Exception&) {
                // ...the database handle is still cleaned up
                std::cout << "Exception caught, but resources are safe" << std::endl;
            }
            // scoped_db is automatically cleaned up here
        }
        std::cout << "Scoped database handle cleaned up" << std::endl;

        // Example 5: Multiple operations with error handling
        std::cout << "\n=== Example 5: Batch error handling ===" << std::endl;
        std::vector<std::string> keys = {"key1", "key2", "key3"};

        for(const auto& k : keys) {
            try {
                std::vector<char> buffer(256);
                size_t size = buffer.size();
                db.get(k.data(), k.size(), buffer.data(), &size);
                std::string value(buffer.data(), size);
                std::cout << k << " = " << value << std::endl;
            } catch(const yokan::Exception&) {
                std::cout << k << " = <not found>" << std::endl;
            }
        }

        std::cout << "\n=== All examples completed ===" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
