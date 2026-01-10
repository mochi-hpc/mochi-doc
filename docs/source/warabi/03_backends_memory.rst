Backend: memory
===============

The memory backend stores data in RAM, providing the fastest access but with
no persistence across provider restarts.

When to use
-----------

Use the memory backend when:

- You need maximum performance
- Data is temporary or can be regenerated
- You're implementing caching
- You're testing or prototyping
- Persistence is not required

Characteristics
---------------

**Fast**: All operations are in-memory with no I/O overhead

**Volatile**: Data is lost when the provider stops

**No size limits**: Limited only by available RAM

**Simple**: No configuration needed

Configuration
-------------

In Bedrock configuration:

.. code-block:: json

   {
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "target": {
                   "type": "memory",
                   "config": {}
               }
           }
       }]
   }

The memory backend has no configuration options - just use an empty :code:`config` object.

In C++ code:

.. code-block:: cpp

   #include <warabi/Provider.hpp>

   auto config = R"(
   {
       "target": {
           "type": "memory",
           "config": {}
       }
   }
   )";

   warabi::Provider provider(engine, 42, config);

Memory management
-----------------

The memory backend allocates memory dynamically as regions are created and
written to:

.. code-block:: cpp

   // Create region (no allocation yet)
   warabi::RegionID id;
   target.create(&id);

   // Write data (memory allocated now)
   std::vector<char> data(1024 * 1024);  // 1 MB
   target.write(id, 0, data.data(), data.size());

Memory is freed when:
- A region is destroyed: :code:`target.erase(id)`
- The provider is shut down
