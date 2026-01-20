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

        // APPEND mode: Append to existing values
        std::string log_key = "application_log";
        std::string entry1 = "Entry 1\n";
        std::string entry2 = "Entry 2\n";
        std::string entry3 = "Entry 3\n";

        db.put(log_key.data(), log_key.size(), entry1.data(), entry1.size());
        db.put(log_key.data(), log_key.size(), entry2.data(), entry2.size(),
               YOKAN_MODE_APPEND);
        db.put(log_key.data(), log_key.size(), entry3.data(), entry3.size(),
               YOKAN_MODE_APPEND);

        std::vector<char> log_buffer(1024);
        size_t log_size = log_buffer.size();
        db.get(log_key.data(), log_key.size(), log_buffer.data(), &log_size);
        std::string log(log_buffer.data(), log_size);
        std::cout << "Appended log:\n" << log << std::endl;

        // CONSUME mode: Get and erase atomically
        std::string task_key = "pending_task";
        std::string task_value = "Process data";
        db.put(task_key.data(), task_key.size(), task_value.data(), task_value.size());

        std::vector<char> task_buffer(256);
        size_t task_size = task_buffer.size();
        db.get(task_key.data(), task_key.size(), task_buffer.data(), &task_size,
               YOKAN_MODE_CONSUME);
        std::string task(task_buffer.data(), task_size);
        std::cout << "Consumed task: " << task << std::endl;

        bool still_exists = db.exists(task_key.data(), task_key.size());
        std::cout << "Task still exists: " << (still_exists ? "yes" : "no") << std::endl;

        // NEW_ONLY mode: Only put if key doesn't exist
        std::string counter_key = "counter";
        std::string initial_value = "1";

        db.put(counter_key.data(), counter_key.size(),
               initial_value.data(), initial_value.size(),
               YOKAN_MODE_NEW_ONLY);
        std::cout << "Initial counter set" << std::endl;

        try {
            std::string new_value = "2";
            db.put(counter_key.data(), counter_key.size(),
                   new_value.data(), new_value.size(),
                   YOKAN_MODE_NEW_ONLY);
            std::cout << "Second put shouldn't succeed!" << std::endl;
        } catch(const yokan::Exception& ex) {
            std::cout << "Expected: Can't overwrite with NEW_ONLY" << std::endl;
        }

        // Combining modes
        std::string multi_mode_key = "combined";
        std::string multi_value = "test";
        db.put(multi_mode_key.data(), multi_mode_key.size(),
               multi_value.data(), multi_value.size());

        std::vector<char> combined_buffer(256);
        size_t combined_size = combined_buffer.size();
        db.get(multi_mode_key.data(), multi_mode_key.size(),
               combined_buffer.data(), &combined_size,
               YOKAN_MODE_CONSUME | YOKAN_MODE_NO_RDMA);
        std::cout << "Used combined modes: CONSUME | NO_RDMA" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
