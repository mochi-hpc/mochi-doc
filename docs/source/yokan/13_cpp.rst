C++ Bindings
=============

Yokan provides comprehensive header-only C++ bindings that wrap the C API
with modern C++ idioms, RAII resource management, and exception handling.

Overview
--------

The C++ bindings are located in ``include/yokan/cxx/`` and provide C++ classes
equivalent to the C opaque pointers:

- ``yokan::Client`` - Wrapper for ``yk_client_t``
- ``yokan::Database`` - Wrapper for ``yk_database_handle_t``
- ``yokan::Collection`` - Wrapper for document collections
- ``yokan::Exception`` - C++ exception for error handling

.. warning::
   Some C++ functions have parameters in a different order than their C
   equivalents. In particular, functions that take a mode have this mode
   as the last parameter to allow C++ optional parameters.

Quick Start
-----------

Here's a minimal example using the C++ API:

.. literalinclude:: ../../../code/yokan/13_cpp/quickstart.cpp
   :language: cpp

This demonstrates:

- Automatic resource management (RAII)
- Exception-based error handling
- Clean, modern C++ API

Client and Database Handles
----------------------------

Creating Clients and Databases
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../code/yokan/13_cpp/client_database.cpp
   :language: cpp

The ``yokan::Client`` and ``yokan::Database`` classes:

- Use RAII for automatic cleanup
- Are copyable (reference counted)
- Are movable for efficient transfer
- Throw ``yokan::Exception`` on errors

Basic Operations
----------------

Put, Get, Exists, and Erase
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../code/yokan/13_cpp/basic_operations.cpp
   :language: cpp

All basic operations:

- Accept ``const void*`` and ``size_t`` for binary data
- Support strings through ``.data()`` and ``.size()``
- Take an optional mode parameter (defaults to ``YOKAN_MODE_DEFAULT``)
- Throw ``yokan::Exception`` on errors

Batch Operations
----------------

Multi-key Operations
~~~~~~~~~~~~~~~~~~~~

For efficiency, use batch operations when working with multiple keys:

.. literalinclude:: ../../../code/yokan/13_cpp/batch_operations.cpp
   :language: cpp

Batch operations reduce network round-trips and improve throughput significantly.

String and Vector Helpers
~~~~~~~~~~~~~~~~~~~~~~~~~~

The C++ API provides convenient helpers for working with ``std::string`` and
``std::vector``:

.. literalinclude:: ../../../code/yokan/13_cpp/string_helpers.cpp
   :language: cpp

List Operations
---------------

The C++ API provides wrappers for list operations that work with callbacks:

.. literalinclude:: ../../../code/yokan/13_cpp/list_operations.cpp
   :language: cpp

Alternatively, use the packed variants for lower-level control:

.. literalinclude:: ../../../code/yokan/13_cpp/list_packed.cpp
   :language: cpp

Working with Modes
------------------

Modes modify operation semantics and can be combined using bitwise OR:

.. literalinclude:: ../../../code/yokan/13_cpp/modes.cpp
   :language: cpp

Available modes include:

- ``YOKAN_MODE_DEFAULT`` - Default behavior
- ``YOKAN_MODE_INCLUSIVE`` - Include lower bound in lists
- ``YOKAN_MODE_APPEND`` - Append to values
- ``YOKAN_MODE_CONSUME`` - Get and erase atomically
- ``YOKAN_MODE_WAIT`` - Wait for keys to appear
- ``YOKAN_MODE_NOTIFY`` - Notify waiting clients
- ``YOKAN_MODE_NEW_ONLY`` - Only put if key doesn't exist
- ``YOKAN_MODE_EXIST_ONLY`` - Only put if key exists
- ``YOKAN_MODE_NO_RDMA`` - Disable RDMA for small data

Exception Handling
------------------

The C++ API throws ``yokan::Exception`` for all errors:

.. literalinclude:: ../../../code/yokan/13_cpp/exceptions.cpp
   :language: cpp

Best practices:

1. Wrap operations in try/catch blocks
2. Use RAII for automatic cleanup
3. Check backend capabilities before using advanced modes
4. Handle missing keys gracefully

RAII and Resource Management
-----------------------------

Copy and Move Semantics
~~~~~~~~~~~~~~~~~~~~~~~~

The C++ classes use reference counting for safe copying:

.. literalinclude:: ../../../code/yokan/13_cpp/raii.cpp
   :language: cpp

Resources are automatically cleaned up when the last reference is destroyed,
preventing leaks and simplifying code.

Advanced Patterns
-----------------

Custom Memory Management
~~~~~~~~~~~~~~~~~~~~~~~~

For high-performance applications, preallocate buffers:

.. literalinclude:: ../../../code/yokan/13_cpp/custom_memory.cpp
   :language: cpp

Binary Data Handling
~~~~~~~~~~~~~~~~~~~~

Working with binary (non-text) data:

.. literalinclude:: ../../../code/yokan/13_cpp/binary_data.cpp
   :language: cpp

Template-Based Wrappers
~~~~~~~~~~~~~~~~~~~~~~~

Create type-safe wrappers using templates:

.. literalinclude:: ../../../code/yokan/13_cpp/templates.cpp
   :language: cpp

Migration API
-------------

The C++ API provides a clean interface for database migration:

.. literalinclude:: ../../../code/yokan/13_cpp/migration.cpp
   :language: cpp

See :doc:`10_migration` for detailed migration documentation.

Document Collections
--------------------

Work with JSON documents using the Collection API:

.. literalinclude:: ../../../code/yokan/13_cpp/collections.cpp
   :language: cpp

Collections provide:

- JSON document storage
- Document IDs
- Batch operations
- Query capabilities

Performance Considerations
--------------------------

1. **Use batch operations** to reduce network round-trips

2. **Preallocate buffers** for get operations when sizes are known

3. **Use move semantics** to avoid unnecessary copying

4. **Choose appropriate backends** based on workload characteristics

5. **Use NO_RDMA mode** for small key/value pairs

6. **Consider pool configuration** for concurrent operations

7. **Profile your application** to identify bottlenecks

Comparison with C API
---------------------

**C++ Advantages:**

- RAII automatic cleanup
- Exception-based error handling
- Type safety
- Modern C++ idioms
- Easier to use correctly

**When to use C API:**

- C-only environments
- ABI stability requirements
- Specific performance needs
- Library interoperability

Integration Examples
--------------------

With Thallium
~~~~~~~~~~~~~

The C++ bindings work seamlessly with Thallium:

.. literalinclude:: ../../../code/yokan/13_cpp/with_thallium.cpp
   :language: cpp

With Bedrock
~~~~~~~~~~~~

Using Yokan with Bedrock's C++ API:

.. literalinclude:: ../../../code/yokan/13_cpp/with_bedrock.cpp
   :language: cpp

See :doc:`02_advanced_setup` for Bedrock configuration details.

Building Applications
---------------------

CMake Integration
~~~~~~~~~~~~~~~~~

Use pkg-config to find Yokan:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.10)
   project(my_yokan_app CXX)

   set(CMAKE_CXX_STANDARD 14)

   find_package(PkgConfig REQUIRED)
   pkg_check_modules(YOKAN REQUIRED IMPORTED_TARGET yokan-client)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PkgConfig::YOKAN)

Manual Compilation
~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   g++ -std=c++14 my_app.cpp $(pkg-config --cflags --libs yokan-client) -o my_app

Next Steps
----------

- Review :doc:`12_python` for Python bindings
- Learn about :doc:`10_migration` for data migration
- Explore :doc:`11_watcher` for event-driven patterns
- See :doc:`05_modes` for all available modes

Summary
-------

The Yokan C++ bindings provide:

- Header-only implementation
- RAII resource management
- Exception-based error handling
- Modern C++ idioms
- Full feature parity with C API

The bindings are designed for ease of use while maintaining high performance,
making Yokan ideal for modern C++ applications in HPC and distributed systems.
