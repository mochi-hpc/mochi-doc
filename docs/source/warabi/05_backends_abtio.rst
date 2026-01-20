Backend: abt-io
===============

The abt-io backend uses ABT-IO (Argobots-aware I/O) for asynchronous file I/O,
providing persistent storage with non-blocking operations.

When to use
-----------

Use the abt-io backend when:

- You need persistent storage on traditional filesystems
- You want async I/O without blocking Argobots threads
- You're storing very large blobs (multi-GB)
- You don't have persistent memory hardware
- You want compatibility with standard filesystems (ext4, xfs, etc.)

Characteristics
---------------

**Persistent**: Data stored on disk survives restarts

**Async**: I/O operations execute in Argobots ULTs potentially in a separate ES

**Large capacity**: Limited by filesystem size


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
                   "type": "abtio",
                   "config": {
                       "path": "/tmp/warabi",
                       "create_if_missing": true,
                       "override_if_exists": true,
                       "alignment": 4096,
                       "directio": false,
                       "abt_io": {
                            
                       }
                   }
               }
           }
       }]
   }

Configuration options:

- :code:`path`: Files where regions will be stored
- :code:`create_if_missing` (default "false"): Whether to create the file if it is missing
- :code:`override_if_exists` (default "false"): Whether to override the file if it exists
- :code:`directio` (default "false"): Whether to open the file with ``O_DIRECT``
- :code:`alignment` (default 8): alignment of regions in the file
- :code:`abt_io`: configuration of an ABT-IO instance (see ABT-IO section for more information)

In C++ code:

.. code-block:: cpp

   #include <warabi/Provider.hpp>

   auto config = R"(
   {
       "target": {
           "type": "abtio",
           "config": {
               "path": "/tmp/warabi",
               "create_if_missing": true,
               "override_if_exists": true,
               "alignment": 4096,
               "directio": false,
               "abt_io": {}
           }
       }
   }
   )";

   warabi::Provider provider(engine, 42, config);
