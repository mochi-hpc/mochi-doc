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

A minimal Bedrock configuration for Flock with the static backend:

.. literalinclude:: ../../../code/flock/11_bedrock/bedrock-config-static.json
   :language: json

Configuration with the centralized backend:

.. literalinclude:: ../../../code/flock/11_bedrock/bedrock-config-centralized.json
   :language: json

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

.. code-block:: console

   $ bedrock na+sm -c bedrock-config-static.json

**Pattern 2: MPI-based static group**

For HPC applications with fixed membership:

.. code-block:: json

   {
       "config": {
           "bootstrap": "mpi",
           "file": "mygroup.flock",
           "group": {"type": "static", "config": {}}
       }
   }

.. code-block:: console

   $ mpirun -n 4 bedrock ofi+tcp -c config.json

All 4 ranks will form a group together.

**Pattern 3: Coordinator + workers**

For elastic services with dynamic membership:

Start coordinator:

.. code-block:: console

   $ bedrock ofi+tcp -c bedrock-config-centralized.json
   [info] Bedrock daemon now running at ofi+tcp://192.168.1.100:1234

Start workers (use the coordinator's address):

.. code-block:: console

   $ bedrock ofi+tcp -c worker.json
   [info] Joined group with 2 members

**Pattern 4: File-based coordination**

When you can't directly communicate addresses:

*Step 1*: Create initial group and save to file using "self" bootstrap.

*Step 2*: Other processes load from the shared file using "file" bootstrap.

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

This allows view persistence and recovery.

**2. Use appropriate backends**:

- Static for fixed HPC jobs
- Centralized for elastic services

**3. Match transport protocols**:

Ensure Bedrock's transport matches Flock addresses:

.. code-block:: console

   $ bedrock ofi+tcp -c config.json  # Use ofi+tcp addresses

**4. Coordinate provider IDs**:

All group members should use the same provider ID.

**5. Handle failures gracefully**:

Use appropriate timeout values for your network.

Next steps
----------

- :doc:`12_cpp`: Learn about the C++ API
- :doc:`08_backends_centralized`: Learn about centralized backend configuration
