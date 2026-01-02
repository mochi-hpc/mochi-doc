C++ API
=======

Flock provides a modern C++ API built on top of Thallium. It offers a more
intuitive interface using C++ features like RAII, exceptions, and STL containers.

Prerequisites
-------------

The C++ API requires Thallium and a C++14 compatible compiler:

.. code-block:: console

   spack install mochi-flock +bedrock
   spack install mochi-thallium

Include headers
---------------

.. code-block:: cpp

   #include <flock/cxx/server.hpp>
   #include <flock/cxx/client.hpp>

Namespaces
----------

All C++ API is in the :code:`flock` namespace:

.. code-block:: cpp

   using namespace flock;

Provider (server-side)
----------------------

**Creating a provider**:

.. code-block:: cpp

   #include <thallium.hpp>
   #include <flock/cxx/server.hpp>

   namespace tl = thallium;

   int main() {
       tl::engine engine("na+sm", THALLIUM_SERVER_MODE);

       // Create provider with self bootstrap and static backend
       flock::Provider provider(
           engine,                    // Thallium engine
           42,                        // Provider ID
           "{ \"group\": { \"type\": \"static\", \"config\": {} } }",  // Config
           tl::pool()                 // Pool (default)
       );

       engine.wait_for_finalize();
       return 0;
   }

The :code:`Provider` class uses RAII - it registers on construction and
deregisters on destruction.

**With custom initial view**:

.. code-block:: cpp

   flock::GroupView initial_view;
   initial_view.addMember(engine.self(), 42);
   initial_view.addMetadata("service", "my_service");

   flock::Provider provider(
       engine, 42, config, tl::pool(), &initial_view
   );

Client
------

**Creating a client**:

.. code-block:: cpp

   #include <thallium.hpp>
   #include <flock/cxx/client.hpp>

   tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

   flock::Client client(engine);

The client is automatically finalized when the :code:`Client` object is destroyed.

**Creating a group handle**:

.. code-block:: cpp

   tl::endpoint server = engine.lookup("na+sm://12345-0");

   flock::GroupHandle group = client.makeGroupHandle(
       server,      // Server endpoint
       42,          // Provider ID
       true         // Auto-refresh
   );

**From a file**:

.. code-block:: cpp

   flock::GroupHandle group = client.makeGroupHandleFromFile(
       "mygroup.flock",
       true  // Auto-refresh
   );

Group view operations
---------------------

The C++ API provides a :code:`GroupView` class:

.. code-block:: cpp

   #include <flock/cxx/group-view.hpp>

   flock::GroupView view;

**Adding members**:

.. code-block:: cpp

   view.addMember("na+sm://12345-0", 42);
   view.addMember("na+sm://12345-1", 42);

**Adding metadata**:

.. code-block:: cpp

   view.addMetadata("service", "my_service");
   view.addMetadata("version", "1.0.0");

**Accessing members**:

.. code-block:: cpp

   size_t count = view.memberCount();

   for(size_t i = 0; i < count; i++) {
       auto member = view.getMember(i);
       std::cout << "Member " << i << ": "
                 << member.address << " (id=" << member.provider_id << ")\n";
   }

Or using iterators:

.. code-block:: cpp

   for(const auto& member : view.members()) {
       std::cout << member.address << " (id=" << member.provider_id << ")\n";
   }

**Accessing metadata**:

.. code-block:: cpp

   for(const auto& [key, value] : view.metadata()) {
       std::cout << key << " = " << value << "\n";
   }

   // Find specific metadata
   std::string version = view.findMetadata("version").value_or("unknown");

**Serialization**:

.. code-block:: cpp

   // To JSON string
   std::string json = view.serialize();

   // From JSON string
   flock::GroupView view2 = flock::GroupView::deserialize(json);

**File I/O**:

.. code-block:: cpp

   // Save to file
   view.saveToFile("mygroup.flock");

   // Load from file
   flock::GroupView view2 = flock::GroupView::loadFromFile("mygroup.flock");

Querying group membership
--------------------------

**Get the view**:

.. code-block:: cpp

   flock::GroupView view = group.getView();

   std::cout << "Group has " << view.memberCount() << " members\n";

**Access without copying**:

.. code-block:: cpp

   {
       auto view_lock = group.accessView();
       const flock::GroupView& view = *view_lock;

       // Use view (read-only)
       std::cout << "Group size: " << view.memberCount() << "\n";

       // View is automatically released when view_lock goes out of scope
   }

**Get specific member**:

.. code-block:: cpp

   auto member = group.getMember(0);  // Get first member
   std::cout << "Address: " << member.address << "\n";

Membership callbacks
--------------------

Register callbacks using lambdas:

.. code-block:: cpp

   group.onMembershipChange([](const flock::GroupView& view) {
       std::cout << "Group membership changed! New size: "
                 << view.memberCount() << "\n";
   });

Or with function pointers:

.. code-block:: cpp

   void membershipChanged(const flock::GroupView& view) {
       std::cout << "Group changed: " << view.memberCount() << " members\n";
   }

   group.onMembershipChange(membershipChanged);

Exception handling
------------------

The C++ API uses exceptions for error handling:

.. code-block:: cpp

   try {
       flock::GroupHandle group = client.makeGroupHandle(server, 42, true);
       flock::GroupView view = group.getView();

       for(const auto& member : view.members()) {
           std::cout << member.address << "\n";
       }

   } catch(const flock::Exception& ex) {
       std::cerr << "Flock error: " << ex.what() << "\n";
       return -1;
   }

Common exception types:

- :code:`flock::Exception`: Base exception class
- :code:`flock::InvalidArgument`: Invalid argument passed
- :code:`flock::NotFound`: Member or resource not found
- :code:`flock::Mercury`: Mercury/RPC error

Complete example
----------------

Here's a complete C++ client example:

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>
   #include <flock/cxx/client.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {
       if(argc != 3) {
           std::cerr << "Usage: " << argv[0]
                     << " <server address> <provider id>\n";
           return -1;
       }

       try {
           // Initialize Thallium
           tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);

           // Create Flock client
           flock::Client client(engine);

           // Lookup server
           tl::endpoint server = engine.lookup(argv[1]);
           uint16_t provider_id = std::atoi(argv[2]);

           // Create group handle
           flock::GroupHandle group = client.makeGroupHandle(
               server, provider_id, true
           );

           // Get group view
           flock::GroupView view = group.getView();

           // Print members
           std::cout << "Group has " << view.memberCount() << " members:\n";
           for(const auto& member : view.members()) {
               std::cout << "  " << member.address
                         << " (provider_id=" << member.provider_id << ")\n";
           }

           // Print metadata
           if(view.metadata().size() > 0) {
               std::cout << "\nMetadata:\n";
               for(const auto& [key, value] : view.metadata()) {
                   std::cout << "  " << key << " = " << value << "\n";
               }
           }

       } catch(const flock::Exception& ex) {
           std::cerr << "Error: " << ex.what() << "\n";
           return -1;
       } catch(const std::exception& ex) {
           std::cerr << "Unexpected error: " << ex.what() << "\n";
           return -1;
       }

       return 0;
   }

Compile:

.. code-block:: console

   $ g++ -std=c++14 -o client client.cpp \
       $(pkg-config --cflags --libs flock-client thallium)

RAII benefits
-------------

The C++ API uses RAII for automatic resource management:

**No manual cleanup**:

.. code-block:: cpp

   {
       flock::Client client(engine);
       flock::GroupHandle group = client.makeGroupHandle(server, 42, true);
       flock::GroupView view = group.getView();

       // Use view...

       // No need to call cleanup functions!
       // Everything is automatically cleaned up when objects go out of scope
   }

**Move semantics**:

Objects can be efficiently moved:

.. code-block:: cpp

   flock::GroupView createView() {
       flock::GroupView view;
       view.addMember("na+sm://12345-0", 42);
       return view;  // Efficient move, not copy
   }

   flock::GroupView my_view = createView();

STL integration
---------------

The C++ API integrates well with STL:

**Storing views in containers**:

.. code-block:: cpp

   std::vector<flock::GroupView> views;
   views.push_back(view1);
   views.push_back(view2);

**Using algorithms**:

.. code-block:: cpp

   auto view = group.getView();
   auto members = view.members();

   // Find member with specific address
   auto it = std::find_if(members.begin(), members.end(),
       [](const auto& m) { return m.provider_id == 42; });

   if(it != members.end()) {
       std::cout << "Found: " << it->address << "\n";
   }

Thread safety
-------------

Like Thallium, the C++ API is thread-safe when using appropriate pools:

.. code-block:: cpp

   tl::pool client_pool = tl::pool::create(tl::pool::access::mpmc);
   flock::Client client(engine, client_pool);

This allows multiple threads to use the same client safely.

Best practices
--------------

**1. Use RAII**: Let objects clean up automatically

**2. Prefer exceptions**: Use try-catch for error handling

**3. Use auto**: Simplify type declarations

.. code-block:: cpp

   auto view = group.getView();  // Instead of flock::GroupView view = ...

**4. Use range-based for loops**:

.. code-block:: cpp

   for(const auto& member : view.members()) {
       // Process member
   }

**5. Use structured bindings** (C++17):

.. code-block:: cpp

   for(const auto& [key, value] : view.metadata()) {
       std::cout << key << " = " << value << "\n";
   }

Next steps
----------

- :ref:`Thallium`: Learn more about Thallium
- :doc:`01_intro`: Review Flock basics
- :doc:`11_bedrock`: Deploy with Bedrock
