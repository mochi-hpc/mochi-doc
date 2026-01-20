Backend: pmem
=============

The pmem (persistent memory) backend stores data in persistent memory devices,
providing near-memory performance with persistence across provider restarts.
It relies on the PMDK (Persistent Memory Development Kit) software, and
was designed for persistent memory devices ((Intel Optane DCPMM, NVDIMM, etc.).

When to use
-----------

Use the pmem backend when:

- You have persistent memory hardware (Intel Optane, etc.)
- You need both high performance and persistence
- Data must survive provider restarts
- You want byte-addressable persistent storage
- Low latency is critical

Characteristics
---------------

**Persistent**: Data survives provider restarts and system reboots

**Fast**: Near-DRAM performance (100s of nanoseconds latency)

**Byte-addressable**: No block-level I/O overhead

**Limited capacity**: Pmem devices are typically smaller than SSDs

**Power-fail safe**: Writes are durable after return

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
                   "type": "pmem",
                   "config": {
                       "path": "/mnt/pmem/warabi.pmem",
                       "create_if_missing_with_size": 10737418240,
                       "override_if_exists": false
                   }
               }
           }
       }]
   }

Configuration options:

- :code:`path`: File where data will be stored
- :code:`create_if_missing_with_size`: If the target path does not exist, create
  is with the specified size (in bytes). This size should be at least 8MB
  (8388608).
- :code:`override_if_exists`: whether to overwrite an existing target and recreate it.

In C++ code:

.. code-block:: cpp

   #include <warabi/Provider.hpp>

   auto config = R"(
   {
       "target": {
           "type": "pmem",
           "config": {
               "path": "/mnt/pmem/warabi.pmem",
               "create_if_missing_with_size": 10737418240
           }
       }
   }
   )";

   warabi::Provider provider(engine, 42, config);

Pool management
---------------

The pmem backend creates a persistent memory pool at the specified path:

.. code-block:: console

   $ ls -lh /mnt/pmem
   -rw-r--r-- 1 user group 10G Dec 31 10:00 warabi.pmem

This pool file persists across provider restarts. When the provider starts again
with the same configuration, it opens the existing pool and all regions are
still available.
