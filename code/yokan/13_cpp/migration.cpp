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
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <source_addr> <source_provider_id>"
                  << " <dest_addr> <dest_provider_id>" << std::endl;
        return 1;
    }

    try {
        tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
        yokan::Client client(engine.get_margo_instance());

        // Connect to source database
        tl::endpoint source_ep = engine.lookup(argv[1]);
        yokan::Database source_db = client.makeDatabaseHandle(
            source_ep.get_addr(), std::atoi(argv[2]));

        // Connect to destination database
        tl::endpoint dest_ep = engine.lookup(argv[3]);
        yokan::Database dest_db = client.makeDatabaseHandle(
            dest_ep.get_addr(), std::atoi(argv[4]));

        // Populate source database with test data
        std::cout << "Populating source database..." << std::endl;
        for(int i = 0; i < 10; i++) {
            std::string key = "key:" + std::to_string(i);
            std::string value = "value:" + std::to_string(i);
            source_db.put(key.data(), key.size(), value.data(), value.size());
        }

        // Manual migration using list and put operations
        std::cout << "\nStarting manual migration..." << std::endl;

        size_t total_migrated = 0;

        // Use iter to iterate and copy
        auto migration_callback = [&](size_t index, const void* key, size_t ksize,
                                       const void* val, size_t vsize) -> yk_return_t {
            // Put key-value pair into destination database
            dest_db.put(key, ksize, val, vsize);
            total_migrated++;
            return YOKAN_SUCCESS;
        };

        // Migrate all keys (empty prefix means all keys)
        std::string prefix = "";
        std::string filter = "";

        source_db.iter(prefix.data(), prefix.size(),
                       filter.data(), filter.size(),
                       0, // count = 0 means all
                       migration_callback);

        std::cout << "Migrated " << total_migrated << " key-value pairs" << std::endl;

        // Verify migration by checking destination database
        std::cout << "\nVerifying migration..." << std::endl;

        for(int i = 0; i < 10; i++) {
            std::string key = "key:" + std::to_string(i);

            // Check if key exists in destination
            bool exists = dest_db.exists(key.data(), key.size());

            if(exists) {
                // Get value from destination
                std::vector<char> buffer(256);
                size_t vsize = buffer.size();
                dest_db.get(key.data(), key.size(), buffer.data(), &vsize);

                std::string value(buffer.data(), vsize);
                std::cout << "  " << key << " = " << value << std::endl;
            } else {
                std::cout << "  " << key << " NOT FOUND!" << std::endl;
            }
        }

        std::cout << "\nMigration verification complete!" << std::endl;

        // Note: For production use with large databases, consider:
        // 1. Batch operations for better performance
        // 2. Error handling and retry logic
        // 3. Progress tracking
        // 4. Using REMI for provider-level migration (requires server access)

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
