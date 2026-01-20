/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <thallium.hpp>
#include <yokan/cxx/database.hpp>
#include <yokan/cxx/client.hpp>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

namespace tl = thallium;

// Example struct for binary data
struct UserRecord {
    uint64_t id;
    char name[32];
    double balance;
    uint32_t age;
};

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

        // Store binary struct
        UserRecord user;
        user.id = 12345;
        std::strncpy(user.name, "Alice Johnson", sizeof(user.name));
        user.balance = 1234.56;
        user.age = 30;

        std::string key = "user:12345";
        db.put(key.data(), key.size(), &user, sizeof(UserRecord));

        std::cout << "Stored binary record" << std::endl;

        // Retrieve binary struct
        UserRecord retrieved_user;
        size_t vsize = sizeof(UserRecord);
        db.get(key.data(), key.size(), &retrieved_user, &vsize);

        std::cout << "Retrieved user:" << std::endl;
        std::cout << "  ID: " << retrieved_user.id << std::endl;
        std::cout << "  Name: " << retrieved_user.name << std::endl;
        std::cout << "  Balance: $" << retrieved_user.balance << std::endl;
        std::cout << "  Age: " << retrieved_user.age << std::endl;

        // Store binary blob (e.g., image data)
        std::vector<uint8_t> blob_data(1024);
        for(size_t i = 0; i < blob_data.size(); i++) {
            blob_data[i] = static_cast<uint8_t>(i % 256);
        }

        std::string blob_key = "image:001";
        db.put(blob_key.data(), blob_key.size(),
               blob_data.data(), blob_data.size());

        std::cout << "\nStored binary blob of " << blob_data.size() << " bytes" << std::endl;

        // Retrieve blob
        std::vector<uint8_t> retrieved_blob(1024);
        size_t blob_size = retrieved_blob.size();
        db.get(blob_key.data(), blob_key.size(),
               retrieved_blob.data(), &blob_size);

        // Verify data
        bool match = (blob_data == retrieved_blob);
        std::cout << "Binary data matches: " << (match ? "yes" : "no") << std::endl;

        // Store array of integers
        std::vector<int32_t> int_array = {10, 20, 30, 40, 50};
        std::string array_key = "array:001";

        db.put(array_key.data(), array_key.size(),
               int_array.data(), int_array.size() * sizeof(int32_t));

        // Retrieve array
        std::vector<int32_t> retrieved_array(5);
        size_t array_size = retrieved_array.size() * sizeof(int32_t);
        db.get(array_key.data(), array_key.size(),
               retrieved_array.data(), &array_size);

        std::cout << "\nRetrieved array: ";
        for(const auto& val : retrieved_array) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
