Bootstrap method: self
======================

The "self" bootstrap method is the simplest way to initialize a Flock group.
It creates a single-member group containing only the current process/provider.

When to use
-----------

Use the "self" bootstrap method when:

- You're starting with a single process and will dynamically add members later
- You're testing or prototyping
- Your service doesn't need initial coordination with other processes

Configuration
-------------

In Bedrock configuration:

.. code-block:: json

   {
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

In C code
---------

When creating a provider programmatically, use :code:`flock_group_view_init_from_self`:

.. code-block:: c

   #include <flock/flock-server.h>
   #include <flock/flock-bootstrap.h>

   // ... margo initialization ...

   struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
   flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
   args.initial_view = &initial_view;

   // Initialize view from self
   flock_group_view_init_from_self(mid, provider_id, &initial_view);

   const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";
   flock_provider_register(mid, provider_id, config, &args, FLOCK_PROVIDER_IGNORE);

The :code:`flock_group_view_init_from_self` function takes:

- The Margo instance
- The provider ID
- A pointer to the group view to initialize

This will populate the view with a single member (the current process).

Group view structure
--------------------

After initialization with "self", the group view contains:

- A single member with the current process's address and provider ID
- Empty metadata
- An initial digest for view versioning

You can persist this view to a file or share it with other processes to allow
them to join the group using the "join" or "view" bootstrap methods.

Next steps
----------

- :doc:`03_bootstrap_view`: Learn about initializing from a provided group view
- :doc:`04_bootstrap_mpi`: Learn about MPI-based bootstrapping for multi-process groups
- :doc:`05_bootstrap_join`: Learn about joining an existing group
