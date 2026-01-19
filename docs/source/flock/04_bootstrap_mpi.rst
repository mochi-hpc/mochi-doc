Bootstrap method: mpi
======================

The "mpi" bootstrap method allows you to initialize a Flock group from an MPI communicator.
This is particularly useful for HPC applications that are already using MPI for process management.

When to use
-----------

Use the "mpi" bootstrap method when:

- Your application is already using MPI
- You want all MPI ranks to automatically form a group
- You need the group membership to match your MPI communicator
- You're deploying on HPC systems with MPI launchers

Prerequisites
-------------

To use the MPI bootstrap method, Flock must be compiled with MPI support:

.. code-block:: console

   spack install mochi-flock +mpi +bedrock

Configuration
-------------

In Bedrock configuration:

.. literalinclude:: ../../../code/flock/04_bootstrap_mpi/bedrock-config.json
   :language: json


When you launch your Bedrock application with MPI (e.g., :code:`mpirun -n 4 bedrock ...`),
all ranks will automatically form a group with each other.

If you want only some of the ranks to be part of the group, for instance ranks 0, 1, 2, and 3,
add the following in the provider's "config" field:

.. code-block:: json

   "mpi_ranks": [0, 1, 2, 3]

In C code
---------

To use MPI bootstrap programmatically, you can rely on the ``flock_group_view_init_from_mpi``
helper function from the ``flock/flock-bootstrap-mpi.h`` header file.

.. literalinclude:: ../../../code/flock/04_bootstrap_mpi/server.c
   :language: c

The :code:`flock_group_view_init_from_mpi` function takes:

- The Margo instance
- The provider ID
- An MPI communicator (typically :code:`MPI_COMM_WORLD`)
- A pointer to the group view to initialize

This function performs an MPI collective operation to gather all addresses and construct
a group view containing all MPI ranks in the specified communicator.

.. important::

   The "mpi_ranks" field is only processed by Bedrock, not when using the C API.
   The reason is that the C API allows users to create exactly the communicator
   they want the group to use, while Bedrock is restricted to using ``MPI_COMM_WORLD``
   and needs a way to specify which ranks in ``MPI_COMM_WORLD`` are concerned.

How it works
------------

The MPI bootstrap process:

1. Each MPI rank determines its Margo address
2. An ``MPI_Allgather`` collective exchanges addresses between all ranks
3. Each rank constructs an identical group view with all members
4. The group is initialized with this view

Because this uses MPI collectives, **all MPI ranks must call the bootstrap function
simultaneously**. If some ranks don't participate, the collective will hang.

Example usage
-------------

Compile your application with MPI support:

.. code-block:: console

   $ mpicc -o server server.c $(pkg-config --cflags --libs flock-server margo)

Launch with mpirun:

.. code-block:: console

   $ mpirun -n 4 ./server
   [Rank 0] Server running with 4 group members
   [Rank 1] Server running with 4 group members
   [Rank 2] Server running with 4 group members
   [Rank 3] Server running with 4 group members

All four processes will have identical group views containing all four members.
