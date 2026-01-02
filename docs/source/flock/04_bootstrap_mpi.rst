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
                   "bootstrap": "mpi",
                   "group": {
                       "type": "static",
                       "config": {}
                   },
                   "file": "mygroup.flock"
               }
           }
       ]
   }

When you launch your Bedrock application with MPI (e.g., :code:`mpirun -n 4 bedrock ...`),
all ranks will automatically form a group with each other.

In C code
---------

To use MPI bootstrap programmatically:

.. code-block:: c

   #include <mpi.h>
   #include <flock/flock-server.h>
   #include <flock/flock-bootstrap.h>

   int main(int argc, char** argv)
   {
       // Initialize MPI
       MPI_Init(&argc, &argv);

       // Initialize Margo
       margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);

       // Initialize provider arguments and view
       struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
       flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
       args.initial_view = &initial_view;

       // Initialize view from MPI communicator
       flock_group_view_init_from_mpi(mid, 42, MPI_COMM_WORLD, &initial_view);

       const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";
       flock_provider_register(mid, 42, config, &args, FLOCK_PROVIDER_IGNORE);

       margo_wait_for_finalize(mid);

       MPI_Finalize();
       return 0;
   }

The :code:`flock_group_view_init_from_mpi` function takes:

- The Margo instance
- The provider ID
- An MPI communicator (typically :code:`MPI_COMM_WORLD`)
- A pointer to the group view to initialize

This function performs an MPI collective operation to gather all addresses and construct
a group view containing all MPI ranks.

How it works
------------

The MPI bootstrap process:

1. Each MPI rank determines its Margo address
2. An MPI_Allgather collective exchanges addresses between all ranks
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

Persisting the group view
--------------------------

You can save the group view to a file after MPI initialization. This allows
non-MPI processes to join the group later using the "file" or "join" bootstrap methods:

.. code-block:: c

   // After initializing the view from MPI
   flock_group_view_init_from_mpi(mid, 42, MPI_COMM_WORLD, &initial_view);

   // Rank 0 writes the view to a file
   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   if (rank == 0) {
       char* json_str = NULL;
       flock_group_view_serialize(&initial_view, &json_str);

       FILE* f = fopen("group.flock", "w");
       fprintf(f, "%s", json_str);
       fclose(f);

       free(json_str);
   }

Next steps
----------

- :doc:`05_bootstrap_join`: Learn about joining an existing group
- :doc:`06_bootstrap_file`: Learn about loading views from files
- :doc:`07_backends_static`: Learn about the static backend for MPI groups
