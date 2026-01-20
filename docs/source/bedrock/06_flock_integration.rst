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

.. literalinclude:: ../../../code/bedrock/06_flock_integration/basic-config.json
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

An extra ``"mpi_ranks": [ ... ]`` field may be used in the configuration
to specify which MPI ranks are part of the group. By default, all the ranks
of ``MPI_COMM_WORLD`` will be part of the group.

Join bootstrap
^^^^^^^^^^^^^^

For dynamic groups where members join an existing group:

.. code-block:: json

   {
       "type": "flock",
       "config": {
           "bootstrap": "join",
           "file": "/path/to/group_file",
           "group": {"type": "centralized", "config": {}}
       }
   }

Bedrock servers using the "join" method will expect the group file to exist
and will join the group identified by the group file.

.. note::
   The "join" method cannot be used with a "static" group.

Using multiple methods
^^^^^^^^^^^^^^^^^^^^^^

The ``"bootstrap"`` field can accept a list of strings instead of a string,
in which case the bootstrap methods will be attempted one after the other until
one works.

Multi-process deployment
-------------------------

Here's a complete example for deploying a distributed key-value store using
Flock and Yokan across multiple processes.

Configuration file (same for all processes)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../code/bedrock/06_flock_integration/distributed-config.json
   :language: json

Launching the service
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: console

   # Launch with MPI
   $ mpirun -n 4 bedrock na+sm -c distributed-config.json

   # Or launch manually with file-based bootstrap
   # Process 1 (creates the group file)
   $ bedrock na+sm -c distributed-config.json

   # Processes 2-4 (join via the group file)
   $ bedrock na+sm -c distributed-config.json

In the case where process 1 is launched before other processes, the use of ``["join", "mpi"]``
as bootstrap method will make process 1 try to join, fail, and resort to "mpi", creating a
group with itself as the only member. Subsequent processes will find the group file present
and successfully use the "join" method.

Service discovery example
^^^^^^^^^^^^^^^^^^^^^^^^^^

Once the service is running, clients can discover all members:

.. literalinclude:: ../../../code/bedrock/06_flock_integration/client.cpp
   :language: cpp

Python integration
------------------

Using Flock with Bedrock Python bindings:

.. literalinclude:: ../../../code/bedrock/06_flock_integration/python_example.py
   :language: python
