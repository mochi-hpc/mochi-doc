/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/client.hpp>
#include <yokan/cxx/database.hpp>
#include <iostream>
#include <string>

namespace tl = thallium;

int main() {
    try {
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create a client using the Margo instance from Thallium
        yokan::Client client(engine.get_margo_instance());

        // Look up the server address
        tl::endpoint server_ep = engine.lookup("na+sm://localhost:1234");

        // Connect to a database using the resolved address
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(), 42);

        // Put a value
        std::string key = "greeting";
        std::string value = "Hello, Yokan!";
        db.put(key.data(), key.size(), value.data(), value.size());

        // Get the value back
        std::vector<char> buffer(256);
        size_t vsize = buffer.size();
        db.get(key.data(), key.size(), buffer.data(), &vsize);

        std::string retrieved(buffer.data(), vsize);
        std::cout << "Retrieved: " << retrieved << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
