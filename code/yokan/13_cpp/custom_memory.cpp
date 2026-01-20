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
#include <string>

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
        yokan::Database db = client.makeDatabaseHandle(
            server_ep.get_addr(), std::atoi(argv[2]));

        // Strategy 1: Preallocate buffer for known size
        std::string key = "large_value";

        // Get the value size first
        size_t value_size = db.length(key.data(), key.size());

        // Preallocate exact size
        std::vector<char> value_buffer(value_size);
        db.get(key.data(), key.size(), value_buffer.data(), &value_size);

        std::cout << "Retrieved " << value_size << " bytes" << std::endl;

        // Strategy 2: Reuse buffer for multiple gets
        std::vector<char> reusable_buffer(4096); // 4KB buffer

        std::vector<std::string> keys_to_fetch = {"user:1", "user:2", "user:3"};

        for(const auto& k : keys_to_fetch) {
            size_t vsize = reusable_buffer.size();
            db.get(k.data(), k.size(), reusable_buffer.data(), &vsize);

            std::string value(reusable_buffer.data(), vsize);
            std::cout << k << " = " << value << std::endl;
        }

        // Strategy 3: Batch operations with preallocated arrays
        const size_t batch_size = 100;

        std::vector<const void*> key_ptrs(batch_size);
        std::vector<size_t> key_sizes(batch_size);
        std::vector<void*> val_ptrs(batch_size);
        std::vector<size_t> val_sizes(batch_size);

        // Preallocate value buffers
        std::vector<std::vector<char>> value_buffers(batch_size);
        for(size_t i = 0; i < batch_size; i++) {
            value_buffers[i].resize(256); // 256 bytes per value
            val_ptrs[i] = value_buffers[i].data();
            val_sizes[i] = value_buffers[i].size();
        }

        // Prepare keys
        std::vector<std::string> batch_keys(batch_size);
        for(size_t i = 0; i < batch_size; i++) {
            batch_keys[i] = "batch:" + std::to_string(i);
            key_ptrs[i] = batch_keys[i].data();
            key_sizes[i] = batch_keys[i].size();
        }

        // Batch get with preallocated memory
        db.getMulti(batch_size, key_ptrs.data(), key_sizes.data(),
                    val_ptrs.data(), val_sizes.data());

        std::cout << "Fetched " << batch_size << " values with preallocated buffers" << std::endl;

        // Strategy 4: Memory pool for high-frequency operations
        struct MemoryPool {
            std::vector<char> buffer;
            size_t used = 0;

            MemoryPool(size_t size) : buffer(size) {}

            char* allocate(size_t n) {
                if(used + n > buffer.size()) {
                    throw std::runtime_error("Pool exhausted");
                }
                char* ptr = buffer.data() + used;
                used += n;
                return ptr;
            }

            void reset() { used = 0; }
        };

        MemoryPool pool(10240); // 10KB pool

        for(int i = 0; i < 10; i++) {
            std::string k = "pool:" + std::to_string(i);
            size_t vsize = 512;
            char* buf = pool.allocate(vsize);

            db.get(k.data(), k.size(), buf, &vsize);

            std::string value(buf, vsize);
            std::cout << "Fetched from pool: " << value << std::endl;
        }

        pool.reset(); // Reuse pool

    } catch(const yokan::Exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    } catch(const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
