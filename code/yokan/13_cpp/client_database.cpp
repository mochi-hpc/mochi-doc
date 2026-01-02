/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/client.hpp>
#include <yokan/cxx/database.hpp>
#include <iostream>

namespace tl = thallium;

int main() {
    try {
        // Initialize Thallium engine
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

        // Create a client using the Margo instance from Thallium
        yokan::Client client(engine.get_margo_instance());
        std::cout << "Client created" << std::endl;

        // Look up the server address
        tl::endpoint server_ep = engine.lookup("na+sm://localhost:1234");

        // Connect to a database by address and provider ID
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(),        // Server address (hg_addr_t)
            42,                          // Provider ID
            true                         // Check validity
        );
        std::cout << "Database handle created" << std::endl;

        // The client and database are reference counted
        // They'll be automatically cleaned up when they go out of scope

        // Copy the database handle (increments reference count)
        yokan::Database db_copy = db;
        std::cout << "Database handle copied" << std::endl;

        // Move the database handle (transfers ownership)
        yokan::Database db_moved = std::move(db_copy);
        std::cout << "Database handle moved" << std::endl;

        // db_copy is now invalid, db_moved and db are valid
        // All will be cleaned up automatically

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
