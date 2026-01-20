/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <iostream>
#include <memory>

namespace tl = thallium;

void demonstrateCopying(yokan::Database db) {
    // Database handle is copied (reference counted)
    // Original handle remains valid
    std::string key = "copy_test";
    std::string value = "copied";
    db.put(key.data(), key.size(), value.data(), value.size());
    std::cout << "Operation in copied handle successful" << std::endl;
    // db is destroyed here, but reference count decremented
}

yokan::Database demonstrateMoving(yokan::Client& client, hg_addr_t addr, uint16_t provider_id) {
    // Create database handle
    yokan::Database db = client.makeDatabaseHandle(addr, provider_id);

    // Return by value uses move semantics
    return db;
    // Original db is moved, not copied
}

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_addr> <provider_id>" << std::endl;
        return 1;
    }

    try {
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        yokan::Client client(engine.get_margo_instance());
        tl::endpoint server_ep = engine.lookup(argv[1]);

        {
            // Create database handle - RAII ensures cleanup
            yokan::Database db = client.makeDatabaseHandle(
                server_ep.get_addr(), std::atoi(argv[2]));

            // Demonstrate copying
            yokan::Database db_copy = db; // Reference counted copy
            demonstrateCopying(db_copy);

            // Both db and db_copy are still valid
            std::cout << "Original handle still valid" << std::endl;

            // Demonstrate moving
            yokan::Database db_moved = demonstrateMoving(
                client, server_ep.get_addr(), std::atoi(argv[2]));

            std::string key = "move_test";
            std::string value = "moved";
            db_moved.put(key.data(), key.size(), value.data(), value.size());

            // Using smart pointers for dynamic allocation
            auto db_ptr = std::make_unique<yokan::Database>(
                client.makeDatabaseHandle(server_ep.get_addr(), std::atoi(argv[2])));

            key = "smart_ptr_test";
            value = "smart pointer";
            db_ptr->put(key.data(), key.size(), value.data(), value.size());

            // All resources will be automatically cleaned up when going out of scope
            // No need for explicit cleanup calls
        }

        std::cout << "All resources automatically cleaned up" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
