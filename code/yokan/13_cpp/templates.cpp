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
#include <sstream>
#include <type_traits>

namespace tl = thallium;

// Type-safe wrapper for Yokan database
template<typename KeyType, typename ValueType>
class TypedDatabase {
private:
    yokan::Database db;

    // Serialize key to binary
    template<typename T>
    std::vector<char> serialize(const T& obj) {
        if constexpr (std::is_same_v<T, std::string>) {
            return std::vector<char>(obj.begin(), obj.end());
        } else {
            std::vector<char> buffer(sizeof(T));
            std::memcpy(buffer.data(), &obj, sizeof(T));
            return buffer;
        }
    }

    // Deserialize value from binary
    template<typename T>
    T deserialize(const std::vector<char>& data) {
        if constexpr (std::is_same_v<T, std::string>) {
            return std::string(data.begin(), data.end());
        } else {
            T obj;
            std::memcpy(&obj, data.data(), sizeof(T));
            return obj;
        }
    }

public:
    TypedDatabase(yokan::Database db) : db(std::move(db)) {}

    void put(const KeyType& key, const ValueType& value) {
        auto key_data = serialize(key);
        auto val_data = serialize(value);
        db.put(key_data.data(), key_data.size(),
               val_data.data(), val_data.size());
    }

    ValueType get(const KeyType& key) {
        auto key_data = serialize(key);

        // Get value size first
        size_t vsize = db.length(key_data.data(), key_data.size());

        // Allocate and get value
        std::vector<char> val_data(vsize);
        db.get(key_data.data(), key_data.size(), val_data.data(), &vsize);

        return deserialize<ValueType>(val_data);
    }

    bool exists(const KeyType& key) {
        auto key_data = serialize(key);
        return db.exists(key_data.data(), key_data.size());
    }

    void erase(const KeyType& key) {
        auto key_data = serialize(key);
        db.erase(key_data.data(), key_data.size());
    }
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

        // Create typed database wrapper for string->string
        TypedDatabase<std::string, std::string> string_db(db);

        string_db.put("name", "Alice");
        std::string name = string_db.get("name");
        std::cout << "Name: " << name << std::endl;

        // Create typed database wrapper for int->double
        TypedDatabase<int, double> numeric_db(db);

        numeric_db.put(42, 3.14159);
        double value = numeric_db.get(42);
        std::cout << "Value for key 42: " << value << std::endl;

        // Check existence
        bool exists = numeric_db.exists(42);
        std::cout << "Key 42 exists: " << (exists ? "yes" : "no") << std::endl;

        // Erase
        numeric_db.erase(42);
        exists = numeric_db.exists(42);
        std::cout << "Key 42 exists after erase: " << (exists ? "yes" : "no") << std::endl;

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
