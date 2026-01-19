C++ API
=======

Flock provides a modern C++ API built on top of Thallium. It offers a more
intuitive interface using C++ features like RAII, exceptions, and STL containers.

Prerequisites
-------------

The C++ API requires Thallium and a C++17 compatible compiler:

.. code-block:: console

   spack install mochi-flock +bedrock
   spack install mochi-thallium

Include headers
---------------

.. code-block:: cpp

   #include <flock/cxx/server.hpp>
   #include <flock/cxx/client.hpp>
   #include <flock/cxx/group.hpp>
   #include <flock/cxx/group-view.hpp>

Namespaces
----------

All C++ API is in the :code:`flock` namespace:

.. code-block:: cpp

   using namespace flock;

Provider (server-side)
----------------------

Here's a complete example of creating a Flock provider:

.. literalinclude:: ../../../code/flock/12_cpp/server.cpp
   :language: cpp

The :code:`Provider` class uses RAII - it registers on construction and
deregisters on destruction.

Client
------

Here's a complete example of using the Flock client:

.. literalinclude:: ../../../code/flock/12_cpp/client.cpp
   :language: cpp

The client is automatically finalized when the :code:`Client` object is destroyed.

Group view operations
---------------------

The C++ API provides a :code:`GroupView` class:

**Adding members**:

.. code-block:: cpp

   flock::GroupView view;
   view.members().add("na+sm://12345-0", 42);
   view.members().add("na+sm://12345-1", 42);

**Adding metadata**:

.. code-block:: cpp

   view.metadata().add("service", "my_service");
   view.metadata().add("version", "1.0.0");

**Accessing members**:

.. code-block:: cpp

   auto members = view.members();
   size_t count = members.count();

   for(size_t i = 0; i < count; i++) {
       auto member = members[i];
       std::cout << "Member " << i << ": "
                 << member.address << " (id=" << member.provider_id << ")\n";
   }

**Accessing metadata**:

.. code-block:: cpp

   auto metadata = view.metadata();
   for(size_t i = 0; i < metadata.count(); i++) {
       auto md = metadata[i];
       std::cout << md.key << " = " << md.value << "\n";
   }

Compiling
---------

Compile with pkg-config:

.. code-block:: console

   $ g++ -std=c++17 -o server server.cpp \
       $(pkg-config --cflags --libs flock-server thallium)

   $ g++ -std=c++17 -o client client.cpp \
       $(pkg-config --cflags --libs flock-client thallium)

RAII benefits
-------------

The C++ API uses RAII for automatic resource management:

**No manual cleanup**:

.. code-block:: cpp

   {
       flock::Client client(engine);
       flock::GroupHandle group = client.makeGroupHandle(server, 42, true);
       flock::GroupView view = group.view();

       // Use view...

       // No need to call cleanup functions!
       // Everything is automatically cleaned up when objects go out of scope
   }

Best practices
--------------

**1. Use RAII**: Let objects clean up automatically

**2. Use auto**: Simplify type declarations

.. code-block:: cpp

   auto view = group.view();  // Instead of flock::GroupView view = ...

**3. Use range-based access**:

.. code-block:: cpp

   auto members = view.members();
   for(size_t i = 0; i < members.count(); i++) {
       // Process member
   }

Next steps
----------

- :doc:`01_intro`: Review Flock basics
- :doc:`11_bedrock`: Deploy with Bedrock
