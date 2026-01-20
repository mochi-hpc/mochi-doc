/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <iostream>
#include <string>

namespace tl = thallium;

// Custom RPC that uses Yokan
void store_user(const tl::request& req, yokan::Database& db,
                const std::string& username, const std::string& email) {
    try {
        std::string key = "user:" + username;
        db.put(key.data(), key.size(), email.data(), email.size());

        std::cout << "Stored user: " << username << " -> " << email << std::endl;
        req.respond(true);

    } catch(const yokan::Exception& ex) {
        std::cerr << "Yokan error: " << ex.what() << std::endl;
        req.respond(false);
    }
}

void get_user(const tl::request& req, yokan::Database& db,
              const std::string& username) {
    try {
        std::string key = "user:" + username;

        // Get email
        std::vector<char> buffer(256);
        size_t vsize = buffer.size();
        db.get(key.data(), key.size(), buffer.data(), &vsize);

        std::string email(buffer.data(), vsize);
        std::cout << "Retrieved user: " << username << " -> " << email << std::endl;

        req.respond(email);

    } catch(const yokan::Exception& ex) {
        std::cerr << "Yokan error: " << ex.what() << std::endl;
        req.respond(std::string(""));
    }
}

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <protocol> <provider_id>" << std::endl;
        return 1;
    }

    try {
        // Initialize Thallium engine in server mode
        tl::engine engine(argv[1], THALLIUM_SERVER_MODE);

        // Create Yokan client using Thallium's Margo instance
        yokan::Client client(engine.get_margo_instance());

        // Create database handle (connecting to local provider)
        yokan::Database db = client.makeDatabaseHandle(
            engine.self().get_addr(), std::atoi(argv[2]));

        std::cout << "Server running at " << engine.self() << std::endl;

        // Register Thallium RPCs that use Yokan
        engine.define("store_user",
                     [&db](const tl::request& req, const std::string& username,
                           const std::string& email) {
                         store_user(req, db, username, email);
                     });

        engine.define("get_user",
                     [&db](const tl::request& req, const std::string& username) {
                         get_user(req, db, username);
                     });

        // Enable remote shutdown
        engine.enable_remote_shutdown();

        // Wait for finalize
        engine.wait_for_finalize();

    } catch(const yokan::Exception& ex) {
        std::cerr << "Yokan error: " << ex.what() << std::endl;
        return 1;
    } catch(const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
