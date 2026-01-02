Using Flock with Bedrock
=========================

Bedrock is Mochi's service bootstrapping framework. It provides a unified way to
configure and deploy Flock providers using JSON configuration files.

Prerequisites
-------------

Install Flock with Bedrock support:

.. code-block:: console

   spack install mochi-flock +bedrock

Basic configuration
-------------------

A minimal Bedrock configuration for Flock:

.. code-block:: json

   {
       "libraries": [
           "libflock-bedrock-module.so"
       ],
       "providers": [
           {
               "type": "flock",
               "name": "my_flock_provider",
               "provider_id": 42,
               "config": {
                   "bootstrap": "self",
                   "group": {
                       "type": "static",
                       "config": {}
                   }
               }
           }
       ]
   }

Configuration options
---------------------

**Provider fields**:

- :code:`type`: Must be "flock"
- :code:`name`: A unique name for this provider
- :code:`provider_id`: Numeric identifier (0-65535)
- :code:`config`: Flock-specific configuration

**Flock configuration fields**:

- :code:`bootstrap`: Bootstrap method ("self", "mpi", "join", "file")
- :code:`file`: Optional file path for persisting/loading the view
- :code:`group`: Backend configuration

**Bootstrap-specific fields**:

For "join" bootstrap:

.. code-block:: json

   {
       "bootstrap": "join",
       "join": {
           "address": "na+sm://12345-0",
           "provider_id": 42
       }
   }

**Backend configuration**:

Static backend:

.. code-block:: json

   {
       "group": {
           "type": "static",
           "config": {}
       }
   }

Centralized backend:

.. code-block:: json

   {
       "group": {
           "type": "centralized",
           "config": {
               "heartbeat_interval_ms": 5000,
               "failure_timeout_ms": 15000
           }
       }
   }

Common deployment patterns
--------------------------

**Pattern 1: Single-node group**

For testing or single-process deployments:

.. code-block:: json

   {
       "libraries": ["libflock-bedrock-module.so"],
       "providers": [{
           "type": "flock",
           "provider_id": 42,
           "config": {
               "bootstrap": "self",
               "file": "mygroup.flock",
               "group": {"type": "static", "config": {}}
           }
       }]
   }

.. code-block:: console

   $ bedrock na+sm -c config.json

**Pattern 2: MPI-based static group**

For HPC applications with fixed membership:

.. code-block:: json

   {
       "libraries": ["libflock-bedrock-module.so"],
       "providers": [{
           "type": "flock",
           "provider_id": 42,
           "config": {
               "bootstrap": "mpi",
               "file": "mygroup.flock",
               "group": {"type": "static", "config": {}}
           }
       }]
   }

.. code-block:: console

   $ mpirun -n 4 bedrock ofi+tcp -c config.json

All 4 ranks will form a group together.

**Pattern 3: Coordinator + workers**

For elastic services with dynamic membership:

*Coordinator configuration* (coordinator.json):

.. code-block:: json

   {
       "libraries": ["libflock-bedrock-module.so"],
       "providers": [{
           "type": "flock",
           "provider_id": 42,
           "config": {
               "bootstrap": "self",
               "file": "service.flock",
               "group": {
                   "type": "centralized",
                   "config": {
                       "heartbeat_interval_ms": 3000,
                       "failure_timeout_ms": 10000
                   }
               }
           }
       }]
   }

*Worker configuration* (worker.json):

.. code-block:: json

   {
       "libraries": ["libflock-bedrock-module.so"],
       "providers": [{
           "type": "flock",
           "provider_id": 42,
           "config": {
               "bootstrap": "join",
               "join": {
                   "address": "$COORDINATOR_ADDRESS",
                   "provider_id": 42
               },
               "group": {"type": "centralized", "config": {}}
           }
       }]
   }

Start coordinator:

.. code-block:: console

   $ bedrock ofi+tcp -c coordinator.json
   [info] Bedrock daemon now running at ofi+tcp://192.168.1.100:1234

Start workers (use the coordinator's address):

.. code-block:: console

   $ COORDINATOR_ADDRESS="ofi+tcp://192.168.1.100:1234"
   $ bedrock ofi+tcp -c worker.json
   [info] Joined group with 2 members

**Pattern 4: File-based coordination**

When you can't directly communicate addresses:

*Step 1*: Create initial group and save to file:

.. code-block:: json

   {
       "config": {
           "bootstrap": "self",
           "file": "/shared/fs/mygroup.flock",
           "group": {"type": "static", "config": {}}
       }
   }

*Step 2*: Other processes load from the shared file:

.. code-block:: json

   {
       "config": {
           "bootstrap": "file",
           "file": "/shared/fs/mygroup.flock",
           "group": {"type": "static", "config": {}}
       }
   }

Integrating with other services
--------------------------------

Flock is often used with other Mochi services. Here's an example with Yokan:

.. code-block:: json

   {
       "libraries": [
           "libflock-bedrock-module.so",
           "libyokan-bedrock-module.so"
       ],
       "providers": [
           {
               "type": "flock",
               "name": "group_manager",
               "provider_id": 1,
               "config": {
                   "bootstrap": "mpi",
                   "file": "group.flock",
                   "group": {"type": "static", "config": {}}
               }
           },
           {
               "type": "yokan",
               "name": "kv_store",
               "provider_id": 2,
               "config": {
                   "databases": [{"type": "map", "name": "mydb"}]
               }
           }
       ]
   }

This creates both a Flock provider for group management and a Yokan provider
for key-value storage in the same process.

Runtime querying
----------------

Bedrock allows runtime queries of Flock status. Use the :code:`bedrock-query` tool:

.. code-block:: console

   $ bedrock-query ofi+tcp://192.168.1.100:1234 -c \
       '{"__get_config__": {"provider": "my_flock_provider"}}'

This returns the current configuration of the Flock provider.

Configuration validation
-------------------------

Bedrock validates Flock configurations at startup. Common errors:

**Missing library**:

.. code-block:: console

   [error] Could not load library libflock-bedrock-module.so

Solution: Ensure Flock is installed with Bedrock support and the library is in
the library path.

**Invalid bootstrap method**:

.. code-block:: json

   {
       "bootstrap": "invalid_method"
   }

.. code-block:: console

   [error] Unknown bootstrap method: invalid_method

Solution: Use one of: "self", "view", "mpi", "join", "file".

**Missing join configuration**:

.. code-block:: json

   {
       "bootstrap": "join"
       // Missing "join" field!
   }

.. code-block:: console

   [error] Join bootstrap requires 'join' configuration

Solution: Provide the join address and provider ID.

Environment variables
---------------------

You can use environment variables in Bedrock configurations:

.. code-block:: json

   {
       "config": {
           "bootstrap": "join",
           "join": {
               "address": "${FLOCK_COORDINATOR}",
               "provider_id": 42
           }
       }
   }

.. code-block:: console

   $ export FLOCK_COORDINATOR="ofi+tcp://192.168.1.100:1234"
   $ bedrock ofi+tcp -c worker.json

Best practices
--------------

**1. Always specify a file path**:

This allows view persistence and recovery:

.. code-block:: json

   {
       "config": {
           "file": "mygroup.flock",
           // ... other config ...
       }
   }

**2. Use appropriate backends**:

- Static for fixed HPC jobs
- Centralized for elastic services

**3. Match transport protocols**:

Ensure Bedrock's transport matches Flock addresses:

.. code-block:: console

   $ bedrock ofi+tcp -c config.json  # Use ofi+tcp addresses

**4. Coordinate provider IDs**:

All group members should use the same provider ID:

.. code-block:: json

   {
       "provider_id": 42  // Same across all members
   }

**5. Handle failures gracefully**:

Use appropriate timeout values for your network:

.. code-block:: json

   {
       "group": {
           "type": "centralized",
           "config": {
               "heartbeat_interval_ms": 5000,
               "failure_timeout_ms": 15000
           }
       }
   }

Next steps
----------

- :doc:`12_cpp`: Learn about the C++ API
- :ref:`Bedrock`: Learn more about Bedrock
- :doc:`08_backends_centralized`: Learn about centralized backend configuration
