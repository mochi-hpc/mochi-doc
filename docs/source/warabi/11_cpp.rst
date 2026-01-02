C++ API
=======

Warabi is primarily a C++ library built on Thallium. This tutorial covers the
C++ API in depth, including modern C++ patterns and best practices.

Headers and namespaces
-----------------------

.. code-block:: cpp

   #include <warabi/Provider.hpp>  // Server-side
   #include <warabi/Client.hpp>    // Client-side

   namespace tl = thallium;
   using namespace warabi;  // Optional

All Warabi classes are in the :code:`warabi` namespace.

Provider (server-side)
----------------------

**Creating a provider**:

.. code-block:: cpp

   #include <thallium.hpp>
   #include <warabi/Provider.hpp>

   int main() {
       // Initialize Thallium engine
       tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

       // Warabi configuration
       auto config = R"(
       {
           "targets": [
               {"type": "memory", "config": {}}
           ]
       }
       )";

       // Create provider (RAII - auto-registers)
       warabi::Provider provider(engine, 42, config);

       std::cout << "Provider running at " << engine.self() << std::endl;

       engine.wait_for_finalize();

       return 0;
   }  // Provider auto-deregisters when destroyed

**Constructor parameters**:

.. code-block:: cpp

   Provider(tl::engine& engine,           // Thallium engine
            uint16_t provider_id,         // Provider ID
            const std::string& config,    // JSON configuration
            const tl::pool& pool = tl::pool())  // Optional Argobots pool

**With custom pool**:

.. code-block:: cpp

   tl::pool my_pool = tl::pool::create(tl::pool::access::mpmc);
   warabi::Provider provider(engine, 42, config, my_pool);

Client
------

**Creating a client**:

.. code-block:: cpp

   #include <warabi/Client.hpp>

   tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
   warabi::Client client(engine);  // RAII - auto-finalizes

**Creating target handles**:

.. code-block:: cpp

   // From address and provider ID
   tl::endpoint server = engine.lookup("na+sm://12345-0");
   warabi::TargetHandle target = client.makeTargetHandle(
       server,      // Server endpoint
       42,          // Provider ID
       0            // Target index
   );

**Alternative construction**:

.. code-block:: cpp

   // From string address
   warabi::TargetHandle target = client.makeTargetHandle(
       "na+sm://12345-0",  // Address as string
       42,                  // Provider ID
       0                    // Target index
   );

Region operations
-----------------

**Creating regions**:

.. code-block:: cpp

   warabi::RegionID region_id;
   target.create(&region_id);

**Writing data**:

.. code-block:: cpp

   std::string data = "Hello, Warabi!";
   target.write(region_id, 0, data.data(), data.size());

   // Or with std::vector
   std::vector<char> vec_data(1024);
   target.write(region_id, 0, vec_data.data(), vec_data.size());

**Reading data**:

.. code-block:: cpp

   std::vector<char> buffer(size);
   target.read(region_id, 0, buffer.data(), buffer.size());

**Querying size**:

.. code-block:: cpp

   size_t region_size = target.size(region_id);

**Destroying regions**:

.. code-block:: cpp

   target.destroy(region_id);

Region ID type
--------------

:code:`warabi::RegionID` is a simple type:

.. code-block:: cpp

   // Can be copied, assigned, compared
   warabi::RegionID id1, id2;
   target.create(&id1);

   id2 = id1;  // Copy

   if(id1 == id2) {
       std::cout << "Same region\n";
   }

   // Can be stored in STL containers
   std::vector<warabi::RegionID> regions;
   regions.push_back(id1);

   std::map<std::string, warabi::RegionID> region_map;
   region_map["checkpoint"] = id1;

Exception handling
------------------

Warabi uses exceptions for error handling:

.. code-block:: cpp

   try {
       target.create(&region_id);
       target.write(region_id, 0, data, size);

   } catch(const warabi::Exception& ex) {
       std::cerr << "Warabi error: " << ex.what() << std::endl;
       // Handle error
   }

**Exception hierarchy**:

.. code-block:: cpp

   warabi::Exception           // Base exception
   ├── InvalidArgument        // Invalid argument passed
   ├── NotFound               // Region not found
   ├── AlreadyExists          // Region already exists
   ├── BackendError           // Backend-specific error
   └── NetworkError           // RPC/network error

**Catching specific exceptions**:

.. code-block:: cpp

   try {
       target.write(region_id, 0, data, size);

   } catch(const warabi::NotFound& ex) {
       std::cerr << "Region not found: " << ex.what() << std::endl;
       // Recreate region
       target.create(&region_id);

   } catch(const warabi::BackendError& ex) {
       std::cerr << "Backend error: " << ex.what() << std::endl;
       // Handle storage failure
   }

RAII and resource management
-----------------------------

Warabi uses RAII throughout:

**Automatic cleanup**:

.. code-block:: cpp

   {
       tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
       warabi::Client client(engine);

       auto target = client.makeTargetHandle(addr, 42, 0);

       // Use target...

       // No cleanup needed - everything auto-destructs
   }  // engine, client, target all cleaned up automatically

**Move semantics**:

.. code-block:: cpp

   warabi::TargetHandle create_target(warabi::Client& client) {
       return client.makeTargetHandle(addr, 42, 0);
   }

   // Efficient move, not copy
   warabi::TargetHandle target = create_target(client);

**No manual cleanup**:

.. code-block:: cpp

   // NO need for this:
   // target.close();    // Not needed
   // client.finalize(); // Not needed

Modern C++ features
-------------------

**Range-based for loops**:

.. code-block:: cpp

   std::vector<warabi::RegionID> regions = /* ... */;

   for(const auto& region : regions) {
       size_t size = target.size(region);
       std::cout << "Region size: " << size << " bytes\n";
   }

**Auto type deduction**:

.. code-block:: cpp

   auto client = warabi::Client(engine);
   auto target = client.makeTargetHandle(addr, 42, 0);
   auto size = target.size(region_id);

**Lambda functions**:

.. code-block:: cpp

   std::vector<warabi::RegionID> regions = /* ... */;

   // Process regions with lambda
   std::for_each(regions.begin(), regions.end(),
       [&target](const auto& region) {
           std::vector<char> data(target.size(region));
           target.read(region, 0, data.data(), data.size());
           process(data);
       });

**Smart pointers**:

.. code-block:: cpp

   // Store targets in smart pointers if needed
   auto target_ptr = std::make_unique<warabi::TargetHandle>(
       client.makeTargetHandle(addr, 42, 0)
   );

   target_ptr->create(&region_id);

STL integration
---------------

**Storing in containers**:

.. code-block:: cpp

   // Vector of targets
   std::vector<warabi::TargetHandle> targets;
   for(int i = 0; i < num_targets; i++) {
       targets.push_back(client.makeTargetHandle(addr, 42, i));
   }

   // Map of region names to IDs
   std::map<std::string, warabi::RegionID> region_catalog;
   region_catalog["checkpoint_1"] = region_id1;
   region_catalog["checkpoint_2"] = region_id2;

**Using algorithms**:

.. code-block:: cpp

   std::vector<warabi::RegionID> regions = /* ... */;

   // Find largest region
   auto largest = std::max_element(regions.begin(), regions.end(),
       [&target](const auto& a, const auto& b) {
           return target.size(a) < target.size(b);
       });

   // Total size of all regions
   size_t total = std::accumulate(regions.begin(), regions.end(), 0ULL,
       [&target](size_t sum, const auto& region) {
           return sum + target.size(region);
       });

Async operations
----------------

**Async write**:

.. code-block:: cpp

   #include <warabi/Async.hpp>

   auto req = target.write_async(region_id, 0, data, size);

   // Do other work...

   req.wait();  // Wait for completion

**Async read**:

.. code-block:: cpp

   std::vector<char> buffer(size);
   auto req = target.read_async(region_id, 0, buffer.data(), buffer.size());

   // Do other work...

   req.wait();

**Multiple async operations**:

.. code-block:: cpp

   std::vector<warabi::AsyncRequest> requests;

   for(const auto& region : regions) {
       auto req = target.write_async(region, 0, data, size);
       requests.push_back(std::move(req));
   }

   // Wait for all
   for(auto& req : requests) {
       req.wait();
   }

Complete example
----------------

Full C++ client application:

.. code-block:: cpp

   #include <iostream>
   #include <vector>
   #include <string>
   #include <thallium.hpp>
   #include <warabi/Client.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {
       if(argc != 3) {
           std::cerr << "Usage: " << argv[0]
                     << " <server address> <provider_id>\n";
           return -1;
       }

       try {
           // Initialize Thallium
           tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
           std::cout << "Client running\n";

           // Create Warabi client
           warabi::Client client(engine);

           // Get target handle
           warabi::TargetHandle target = client.makeTargetHandle(
               argv[1], std::atoi(argv[2]), 0
           );

           // Create region
           warabi::RegionID region_id;
           target.create(&region_id);
           std::cout << "Created region: " << region_id << "\n";

           // Write data
           std::string message = "Hello from C++ client!";
           target.write(region_id, 0, message.data(), message.size());
           std::cout << "Wrote " << message.size() << " bytes\n";

           // Read back
           size_t size = target.size(region_id);
           std::vector<char> buffer(size);
           target.read(region_id, 0, buffer.data(), buffer.size());

           std::string result(buffer.begin(), buffer.end());
           std::cout << "Read: " << result << "\n";

           // Verify
           if(result == message) {
               std::cout << "SUCCESS\n";
           }

           // Clean up
           target.destroy(region_id);

       } catch(const warabi::Exception& ex) {
           std::cerr << "Warabi error: " << ex.what() << "\n";
           return -1;
       } catch(const std::exception& ex) {
           std::cerr << "Error: " << ex.what() << "\n";
           return -1;
       }

       return 0;
   }

Compile:

.. code-block:: console

   $ g++ -std=c++14 -o client client.cpp \\
       $(pkg-config --cflags --libs warabi-client thallium)

   $ ./client na+sm://12345-0 42
   Client running
   Created region: 12345
   Wrote 22 bytes
   Read: Hello from C++ client!
   SUCCESS

Thread safety
-------------

Warabi is thread-safe when using Thallium pools properly:

.. code-block:: cpp

   tl::pool client_pool = tl::pool::create(tl::pool::access::mpmc);
   warabi::Client client(engine, client_pool);

   // Now safe to use client from multiple threads/ULTs

Best practices
--------------

**1. Use RAII**: Let objects manage their own lifetimes

.. code-block:: cpp

   {
       warabi::Client client(engine);
       // No need to call cleanup
   }  // Auto-cleaned up

**2. Prefer auto**: Simplify code with type deduction

.. code-block:: cpp

   auto target = client.makeTargetHandle(addr, 42, 0);

**3. Use exceptions**: Don't check return codes, use try-catch

**4. Move, don't copy**: Use move semantics for efficiency

.. code-block:: cpp

   auto target = std::move(other_target);  // Move
   // Not: auto target = other_target;     // Copy (less efficient)

**5. Use STL**: Leverage STL containers and algorithms

**6. Modern C++ (C++14/17)**: Use modern language features

.. code-block:: cpp

   // Structured bindings (C++17)
   auto [id1, id2] = create_regions();

   // if-init statement (C++17)
   if(auto size = target.size(region_id); size > threshold) {
       // Handle large region
   }

Next steps
----------

- :ref:`Thallium`: Learn more about Thallium
- :doc:`01_intro`: Review Warabi basics
- :doc:`09_async`: Advanced async patterns in C++
