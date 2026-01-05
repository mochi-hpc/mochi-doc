/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <yokan/cxx/collection.hpp>
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
        uint16_t provider_id = std::atoi(argv[2]);

        // Create database handle
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(), provider_id);

        // Create a collection
        // Note: Collection constructor takes name first, then Database object
        std::string collection_name = "users";
        yokan::Collection collection(collection_name.c_str(), db);

        std::cout << "Working with collection: " << collection_name << std::endl;

        // Prepare JSON documents
        std::vector<std::string> documents = {
            R"({"name": "Alice Johnson", "age": 30, "email": "alice@example.com", "role": "engineer"})",
            R"({"name": "Bob Smith", "age": 35, "email": "bob@example.com", "role": "manager"})",
            R"({"name": "Carol White", "age": 28, "email": "carol@example.com", "role": "engineer"})"
        };

        // Prepare IDs for storing
        std::vector<yk_id_t> ids = {1001, 1002, 1003};

        // Prepare pointers for storeMulti
        std::vector<const void*> doc_ptrs;
        std::vector<size_t> doc_sizes;

        for(const auto& doc : documents) {
            doc_ptrs.push_back(doc.data());
            doc_sizes.push_back(doc.size());
        }

        // Store multiple documents with explicit IDs
        collection.storeMulti(documents.size(),
                             doc_ptrs.data(),
                             doc_sizes.data(),
                             ids.data());

        std::cout << "\nStored " << documents.size() << " documents" << std::endl;
        for(size_t i = 0; i < ids.size(); i++) {
            std::cout << "  ID " << ids[i] << std::endl;
        }

        // Load a document by ID
        yk_id_t id_to_load = 1001;
        std::vector<char> buffer(512);
        size_t doc_size = buffer.size();

        collection.load(id_to_load, buffer.data(), &doc_size);
        std::string retrieved_doc(buffer.data(), doc_size);

        std::cout << "\nRetrieved document " << id_to_load << ":" << std::endl;
        std::cout << retrieved_doc << std::endl;

        // Get document length
        size_t length = collection.length(id_to_load);
        std::cout << "\nDocument " << id_to_load << " length: " << length << " bytes" << std::endl;

        // Update a document
        std::string updated_doc = R"({"name": "Alice Johnson", "age": 31, "email": "alice.j@example.com", "role": "senior engineer"})";

        collection.update(id_to_load, updated_doc.data(), updated_doc.size());
        std::cout << "\nUpdated document " << id_to_load << std::endl;

        // Load updated document
        doc_size = buffer.size();
        collection.load(id_to_load, buffer.data(), &doc_size);
        std::string updated_retrieved(buffer.data(), doc_size);
        std::cout << "Updated document content: " << updated_retrieved << std::endl;

        // Erase a document
        yk_id_t id_to_erase = 1002;
        collection.erase(id_to_erase);
        std::cout << "\nErased document " << id_to_erase << std::endl;

        // List documents using iter with C++ callback
        std::cout << "\nListing all remaining documents:" << std::endl;

        auto callback = [](size_t index, yk_id_t id, const void* doc, size_t size) -> yk_return_t {
            std::string document(static_cast<const char*>(doc), size);
            std::cout << "  [" << index << "] ID " << id << ": " << document << std::endl;
            return YOKAN_SUCCESS;
        };

        yk_id_t start_id = 0; // Start from beginning
        std::string filter = ""; // No filter

        collection.iter(start_id,
                       filter.data(), filter.size(),
                       0, // max = 0 means all
                       callback);

        std::cout << "\nCollection operations completed successfully!" << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
