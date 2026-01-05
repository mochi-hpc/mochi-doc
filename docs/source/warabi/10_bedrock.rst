Using Warabi with Bedrock
==========================

Bedrock is Mochi's service bootstrapping framework. It provides a unified way to
configure and deploy Warabi providers using JSON configuration files.

Prerequisites
-------------

Install Warabi with Bedrock support:

.. code-block:: console

   spack install mochi-warabi +bedrock

Basic configuration
-------------------

A minimal Bedrock configuration for Warabi:

.. code-block:: json

   {
       "libraries": {
           "warabi": "libwarabi-bedrock-module.so"
       },
       "providers": [
           {
               "type": "warabi",
               "name": "my_warabi_provider",
               "provider_id": 42,
               "config": {
                   "targets": [
                       {
                           "type": "memory",
                           "config": {}
                       }
                   ]
               }
           }
       ]
   }

Launch with Bedrock:

.. code-block:: console

   $ bedrock na+sm -c config.json
   [info] [warabi] Warabi provider registered
   [info] Bedrock daemon now running at na+sm://12345-0

Configuration options
---------------------

**Provider fields**:

- :code:`type`: Must be "warabi"
- :code:`name`: A unique name for this provider
- :code:`provider_id`: Numeric identifier (0-65535)
- :code:`config`: Warabi-specific configuration

**Warabi configuration fields**:

- :code:`targets`: Array of target configurations
- :code:`transfer_manager`: Optional transfer manager configuration

Multiple targets
----------------

Configure multiple targets with different backends:

.. code-block:: json

   {
       "config": {
           "targets": [
               {
                   "type": "memory",
                   "config": {}
               },
               {
                   "type": "pmem",
                   "config": {
                       "path": "/mnt/pmem/warabi",
                       "size": 10737418240
                   }
               },
               {
                   "type": "abt-io",
                   "config": {
                       "path": "/data/warabi",
                       "num_threads": 4
                   }
               }
           ]
       }
   }

Access each target by index:

.. code-block:: cpp

   warabi::TargetHandle mem = client.makeTargetHandle(addr, 42, 0);   // memory
   warabi::TargetHandle pmem = client.makeTargetHandle(addr, 42, 1);  // pmem
   warabi::TargetHandle disk = client.makeTargetHandle(addr, 42, 2);  // abt-io

Backend-specific configurations
--------------------------------

**Memory backend**:

.. code-block:: json

   {
       "type": "memory",
       "config": {}
   }

**Pmem backend**:

.. code-block:: json

   {
       "type": "pmem",
       "config": {
           "path": "/mnt/pmem/warabi",
           "size": 10737418240
       }
   }

**ABT-IO backend**:

.. code-block:: json

   {
       "type": "abt-io",
       "config": {
           "path": "/data/warabi",
           "num_threads": 8
       }
   }

**Dummy backend** (for testing):

.. code-block:: json

   {
       "type": "dummy",
       "config": {}
   }

Transfer manager configuration
-------------------------------

**Default transfer manager** (used if not specified):

.. code-block:: json

   {
       "config": {
           "targets": [/*...*/]
           // No transfer_manager = use default
       }
   }

**Pipeline transfer manager**:

.. code-block:: json

   {
       "config": {
           "targets": [/*...*/],
           "transfer_manager": {
               "type": "pipeline",
               "config": {
                   "num_threads": 4,
                   "pipeline_size": 8388608
               }
           }
       }
   }

Common deployment patterns
--------------------------

**Pattern 1: Fast caching layer**

In-memory caching with persistent backup:

.. code-block:: json

   {
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "targets": [
                   {"type": "memory", "config": {}},
                   {"type": "abt-io", "config": {
                       "path": "/data/warabi",
                       "num_threads": 4
                   }}
               ]
           }
       }]
   }

Usage:

.. code-block:: cpp

   // Hot data in memory (target 0)
   memory_target.create(&region_id);
   memory_target.write(region_id, 0, data, size);

   // Cold data or backup on disk (target 1)
   disk_target.create(&backup_id);
   disk_target.write(backup_id, 0, data, size);

**Pattern 2: Persistent memory storage**

High-performance persistent storage:

.. code-block:: json

   {
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "targets": [{
                   "type": "pmem",
                   "config": {
                       "path": "/mnt/pmem/warabi",
                       "size": 107374182400
                   }
               }],
               "transfer_manager": {
                   "type": "pipeline",
                   "config": {
                       "num_threads": 8,
                       "pipeline_size": 16777216
                   }
               }
           }
       }]
   }

**Pattern 3: Large-scale blob storage**

Multiple targets for load distribution:

.. code-block:: json

   {
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "targets": [
                   {"type": "abt-io", "config": {
                       "path": "/data/warabi/shard0",
                       "num_threads": 4
                   }},
                   {"type": "abt-io", "config": {
                       "path": "/data/warabi/shard1",
                       "num_threads": 4
                   }},
                   {"type": "abt-io", "config": {
                       "path": "/data/warabi/shard2",
                       "num_threads": 4
                   }},
                   {"type": "abt-io", "config": {
                       "path": "/data/warabi/shard3",
                       "num_threads": 4
                   }}
               ]
           }
       }]
   }

Distribute data across targets for parallelism.

Multi-node deployment
---------------------

Deploy Warabi across multiple nodes with consistent configuration:

**Node 1 config** (node1.json):

.. code-block:: json

   {
       "libraries": {"warabi": "libwarabi-bedrock-module.so"},
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "targets": [{
                   "type": "abt-io",
                   "config": {
                       "path": "/local/storage/node1",
                       "num_threads": 8
                   }
               }]
           }
       }]
   }

**Node 2 config** (node2.json):

.. code-block:: json

   {
       "libraries": {"warabi": "libwarabi-bedrock-module.so"},
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "targets": [{
                   "type": "abt-io",
                   "config": {
                       "path": "/local/storage/node2",
                       "num_threads": 8
                   }
               }]
           }
       }]
   }

Launch on each node:

.. code-block:: console

   # Node 1
   $ bedrock ofi+tcp://192.168.1.1:1234 -c node1.json

   # Node 2
   $ bedrock ofi+tcp://192.168.1.2:1234 -c node2.json

Clients connect to both nodes for distributed storage.

Integrating with other services
--------------------------------

Warabi often works with other Mochi services:

**With Yokan** (metadata + blob storage):

.. code-block:: json

   {
       "libraries": {
           "yokan": "libyokan-bedrock-module.so",
           "warabi": "libwarabi-bedrock-module.so"
       },
       "providers": [
           {
               "type": "yokan",
               "name": "metadata",
               "provider_id": 1,
               "config": {
                   "databases": [{"type": "map", "name": "region_catalog"}]
               }
           },
           {
               "type": "warabi",
               "name": "blob_storage",
               "provider_id": 2,
               "config": {
                   "targets": [{"type": "abt-io", "config": {
                       "path": "/data/blobs",
                       "num_threads": 4
                   }}]
               }
           }
       ]
   }

Use Yokan to store region IDs and metadata, Warabi for the actual blob data.

**With Flock** (group-aware storage):

.. code-block:: json

   {
       "libraries": {
           "flock": "libflock-bedrock-module.so",
           "warabi": "libwarabi-bedrock-module.so"
       },
       "providers": [
           {
               "type": "flock",
               "provider_id": 1,
               "config": {
                   "bootstrap": "mpi",
                   "group": {"type": "static", "config": {}}
               }
           },
           {
               "type": "warabi",
               "provider_id": 2,
               "config": {
                   "targets": [{"type": "memory", "config": {}}]
               }
           }
       ]
   }

Use Flock to manage group membership, Warabi for distributed blob storage.

Runtime querying
----------------

Query Warabi configuration at runtime using bedrock-query:

.. code-block:: console

   $ bedrock-query ofi+tcp://192.168.1.1:1234 \\
       '{"__get_config__": {"provider": "my_warabi_provider"}}'

This returns the current Warabi provider configuration.

Environment variables
---------------------

Use environment variables in configurations:

.. code-block:: json

   {
       "config": {
           "targets": [{
               "type": "abt-io",
               "config": {
                   "path": "${WARABI_STORAGE_PATH}",
                   "num_threads": "${WARABI_IO_THREADS:4}"
               }
           }]
       }
   }

Launch with environment variables:

.. code-block:: console

   $ export WARABI_STORAGE_PATH="/data/warabi"
   $ export WARABI_IO_THREADS=8
   $ bedrock ofi+tcp -c config.json

Configuration validation
-------------------------

Bedrock validates configurations at startup:

**Missing library**:

.. code-block:: console

   [error] Could not load library libwarabi-bedrock-module.so

Solution: Install Warabi with Bedrock support.

**Invalid backend type**:

.. code-block:: json

   {"type": "invalid_backend"}

.. code-block:: console

   [error] Unknown target type: invalid_backend

Solution: Use "memory", "pmem", "abt-io", or "dummy".

**Missing required fields**:

.. code-block:: json

   {
       "type": "pmem",
       "config": {}  // Missing "path" and "size"
   }

.. code-block:: console

   [error] Pmem backend requires 'path' and 'size' in config

Best practices
--------------

**1. Use appropriate backends**:
- Memory for temporary/cache
- Pmem for fast persistent
- ABT-IO for large persistent

**2. Configure transfer managers**:
- Default for small objects
- Pipeline for large blobs

**3. Multiple targets for workload separation**:

.. code-block:: json

   {
       "targets": [
           {"type": "memory", "config": {}},      // Hot data
           {"type": "pmem", "config": {...}},     // Warm data
           {"type": "abt-io", "config": {...}}    // Cold data
       ]
   }

**4. Match transport to network**:

.. code-block:: console

   # For InfiniBand
   $ bedrock ofi+verbs -c config.json

   # For Ethernet
   $ bedrock ofi+tcp -c config.json

   # For shared memory
   $ bedrock na+sm -c config.json

**5. Monitor and tune**:

Test different configurations and measure performance.

Next steps
----------

- :doc:`11_c_api`: Learn about the C API
- :ref:`Bedrock`: Learn more about Bedrock features
- :doc:`06_transfer_managers`: Optimize transfer performance
