Flock integration
=================

Flock is Mochi's group management service for distributed applications.
When building distributed services with Bedrock, Flock enables:

- Service discovery across multiple processes
- Dynamic membership management
- Group-aware service deployment
- Fault tolerance and elasticity

This tutorial shows how to integrate Flock with Bedrock for building robust
distributed services.

Configuring Flock in Bedrock
-----------------------------

To use Flock in a Bedrock configuration, you need to:

1. Load the Flock module
2. Create a Flock provider
3. Configure the bootstrap method

Basic Flock configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../code/bedrock/08_flock_integration/basic-config.json
   :language: json

This configuration creates a Flock provider that:

- Uses the "self" bootstrap method (single-member group initially)
- Uses the "static" backend (fixed membership)
- Persists the group view to a file

Bootstrap methods
-----------------

Flock supports multiple bootstrap methods for initializing groups.

Self bootstrap
^^^^^^^^^^^^^^

Creates a single-member group with just the current process:

.. code-block:: json

   {
       "type": "flock",
       "config": {
           "bootstrap": "self",
           "group": {"type": "static", "config": {}}
       }
   }

View file bootstrap
^^^^^^^^^^^^^^^^^^^

Initializes from a pre-existing group view file:

.. code-block:: json

   {
       "type": "flock",
       "config": {
           "bootstrap": {
               "method": "view",
               "view_file": "/path/to/group.view"
           },
           "group": {"type": "static", "config": {}}
       }
   }

MPI bootstrap
^^^^^^^^^^^^^

Creates a group from all MPI processes:

.. code-block:: json

   {
       "type": "flock",
       "config": {
           "bootstrap": "mpi",
           "group": {"type": "static", "config": {}}
       }
   }

Then launch with MPI:

.. code-block:: console

   $ mpirun -n 4 bedrock na+sm -c config.json

Join method
^^^^^^^^^^^

For dynamic groups where members join an existing group:

.. code-block:: json

   {
       "type": "flock",
       "config": {
           "bootstrap": {
               "method": "join",
               "address": "na+sm://12345/0",
               "provider_id": 1
           },
           "group": {"type": "centralized", "config": {}}
       }
   }

.. note::
   The "join" method requires the "centralized" backend for dynamic membership.

Using Flock as a dependency
----------------------------

Other providers can depend on the Flock group for service discovery:

.. literalinclude:: ../../../code/bedrock/08_flock_integration/with-dependency.json
   :language: json

The Yokan provider now has access to the Flock group and can:

- Discover other Yokan providers in the group
- Coordinate distributed operations
- Handle membership changes

Multi-process deployment
-------------------------

Here's a complete example for deploying a distributed key-value store using
Flock and Yokan across multiple processes.

Configuration file (same for all processes)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../code/bedrock/08_flock_integration/distributed-config.json
   :language: json

Launching the service
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: console

   # Launch with MPI
   $ mpirun -n 4 bedrock na+sm -c distributed-config.json

   # Or launch manually with file-based bootstrap
   # Process 1 (creates the group file)
   $ bedrock na+sm -c config-file-bootstrap.json

   # Processes 2-4 (join via the group file)
   $ bedrock na+sm -c config-file-bootstrap.json

Service discovery example
^^^^^^^^^^^^^^^^^^^^^^^^^^

Once the service is running, clients can discover all members:

.. literalinclude:: ../../../code/bedrock/08_flock_integration/client.cpp
   :language: cpp

Dynamic membership
------------------

For services that need to handle members joining and leaving, use the
"centralized" backend:

.. code-block:: json

   {
       "name": "dynamic_group",
       "type": "flock",
       "provider_id": 1,
       "config": {
           "bootstrap": "self",
           "group": {
               "type": "centralized",
               "config": {
                   "update_interval": 1000
               }
           }
       }
   }

New members can join at any time:

.. code-block:: json

   {
       "config": {
           "bootstrap": {
               "method": "join",
               "address": "<coordinator_address>",
               "provider_id": 1
           },
           "group": {"type": "centralized", "config": {}}
       }
   }

Python integration
------------------

Using Flock with Bedrock Python bindings:

.. literalinclude:: ../../../code/bedrock/08_flock_integration/python_example.py
   :language: python

Best practices
--------------

1. **Choose the right bootstrap method**:
   - Use "mpi" for HPC batch jobs
   - Use "self" + "join" for elastic services
   - Use "view" for static, pre-configured deployments

2. **Choose the right backend**:
   - Use "static" for fixed membership (better performance)
   - Use "centralized" for dynamic membership

3. **Persist group views**:
   - Always specify a "file" in the config to persist the group view
   - This enables recovery after crashes

4. **Handle membership changes**:
   - If using dynamic membership, implement proper handling of join/leave events
   - Use group refresh callbacks when available

5. **Security**:
   - Protect group view files (they contain service addresses)
   - Consider using authentication in production

Troubleshooting
---------------

**Issue**: Group members can't find each other

**Solutions**:
- Check that all members use the same provider_id
- Verify network connectivity between processes
- Ensure group view files are accessible
- Check firewall settings

**Issue**: "Join" method fails

**Solutions**:
- Verify the coordinator address is correct
- Ensure the coordinator is using "centralized" backend
- Check that the coordinator is running
- Verify provider_id matches

**Issue**: Group view file not updated

**Solutions**:
- Check file permissions
- Verify the "file" path in configuration
- Ensure sufficient disk space

Next steps
----------

- :doc:`../flock`: Learn more about Flock's features and capabilities
- :doc:`05_python`: Use Flock with Bedrock Python bindings
- :doc:`07_runtime_config`: Dynamically add Flock providers at runtime
