Runtime configuration (C++)
============================

While Bedrock configurations are typically provided at startup via JSON files,
Bedrock also provides a powerful C++ API for manipulating configurations at runtime.
This allows you to:

- Add and remove Argobots pools and execution streams
- Load modules dynamically
- Register new providers
- Change provider pools
- Migrate provider state between services
- Snapshot and restore provider state

This tutorial covers how to use these capabilities from C++ code.

Client and ServiceHandle
-------------------------

To manipulate a Bedrock service at runtime, you need to create a :code:`bedrock::Client`
and obtain a :code:`bedrock::ServiceHandle` to the target service.

.. literalinclude:: ../../../code/bedrock/07_runtime_config/basic_client.cpp
   :language: cpp

The :code:`ServiceHandle` is your interface for all runtime configuration operations.

Loading modules at runtime
---------------------------

You can load new Bedrock modules (shared libraries) into a running service:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/load_module.cpp
   :language: cpp

This is useful for dynamically extending a service's capabilities without
requiring a restart.

Managing pools and execution streams
-------------------------------------

You can add and remove Argobots pools and execution streams at runtime.

Adding a pool
^^^^^^^^^^^^^

.. literalinclude:: ../../../code/bedrock/07_runtime_config/add_pool.cpp
   :language: cpp

The pool configuration is provided as a JSON string. After adding a pool,
it becomes available for use by providers and execution streams.

Adding an execution stream
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../code/bedrock/07_runtime_config/add_xstream.cpp
   :language: cpp

Removing pools and execution streams
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   // Remove a pool (must not be in use)
   service.removePool("my_dynamic_pool");

   // Remove an execution stream
   service.removeXstream("my_dynamic_xstream");

.. warning::
   You cannot remove a pool or execution stream that is currently in use
   by a provider or scheduler. Ensure all dependents have been removed first.

Adding providers at runtime
----------------------------

One of the most powerful features is the ability to register new providers
dynamically:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/add_provider.cpp
   :language: cpp

The provider description follows the same format as in the JSON configuration file.
Dependencies can reference existing pools, execution streams, and other providers.

Changing provider pools
------------------------

You can change which Argobots pool a provider uses for handling RPCs:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/change_pool.cpp
   :language: cpp

This is useful for dynamically adjusting resource allocation and load balancing.

.. note::
   Not all providers support changing their pool at runtime. Check the
   provider's documentation to see if this feature is supported.

Provider migration
------------------

Bedrock supports migrating provider state from one service to another:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/migrate.cpp
   :language: cpp

Migration is useful for:

- Load balancing across services
- Draining a service before maintenance
- Elastic scaling of services

The :code:`migration_config` is provider-specific and controls how migration
is performed. The :code:`remove_source` flag determines whether the source
provider's state is deleted after migration.

.. note::
   Provider migration requires that:

   1. The destination address has a provider of the same type
   2. The provider supports migration (implements the :code:`migrate` method)
   3. The migration configuration is valid for the provider type

Snapshotting provider state
----------------------------

You can snapshot a provider's state to persistent storage:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/snapshot.cpp
   :language: cpp

Snapshots can be used for:

- Checkpointing service state
- Backing up data
- Cloning provider state

Restoring from snapshots
^^^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../code/bedrock/07_runtime_config/restore.cpp
   :language: cpp

Asynchronous operations
------------------------

All runtime configuration operations support asynchronous execution using
the :code:`bedrock::AsyncRequest` class:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/async.cpp
   :language: cpp

Asynchronous operations allow your code to continue working while the
configuration change is being applied.

Querying configuration
----------------------

You can retrieve the current configuration of a service:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/query.cpp
   :language: cpp

This returns the complete configuration as a JSON object, which you can
parse and inspect.

Using Jx9 for selective queries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For more complex queries, use Jx9 scripts to extract specific information:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/jx9_query.cpp
   :language: cpp

Complete example
----------------

Here's a complete example that demonstrates multiple runtime operations:

.. literalinclude:: ../../../code/bedrock/07_runtime_config/complete.cpp
   :language: cpp

Compilation
-----------

To compile code using the Bedrock client API, link against the :code:`bedrock::client`
CMake target:

.. code-block:: cmake

   find_package(bedrock REQUIRED)

   add_executable(my_client my_client.cpp)
   target_link_libraries(my_client bedrock::client)

Or use pkg-config:

.. code-block:: console

   $ g++ -std=c++17 my_client.cpp -o my_client \\
       $(pkg-config --cflags --libs bedrock-client)

Error handling
--------------

The Bedrock client API throws :code:`bedrock::Exception` on errors:

.. code-block:: cpp

   #include <bedrock/Exception.hpp>

   try {
       service.addProvider(description);
   } catch (const bedrock::Exception& ex) {
       std::cerr << "Bedrock error: " << ex.what() << std::endl;
   }

Always wrap Bedrock operations in try-catch blocks for robust error handling.

Best practices
--------------

1. **Verify dependencies**: Before adding a provider, ensure all its dependencies
   (pools, other providers) exist.

2. **Handle migration failures**: Provider migration can fail for various reasons
   (network issues, incompatible providers). Always check for exceptions.

3. **Use asynchronous operations**: For operations that may take time (like
   migration or loading large modules), use async requests to avoid blocking.

4. **Clean up carefully**: When removing pools or xstreams, ensure no providers
   are using them. Query the configuration first to check dependencies.

5. **Validate JSON**: When providing JSON configuration strings, ensure they're
   well-formed to avoid parse errors.

Next steps
----------

- :doc:`05_python`: Learn about Python bindings for similar operations
- :doc:`08_flock_integration`: Learn how to use Flock for service group management
- :doc:`04_module`: Learn how to write Bedrock modules that support runtime operations
