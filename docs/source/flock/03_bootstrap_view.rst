Bootstrap method: view
=======================

The "view" bootstrap method allows you to initialize a Flock group from a pre-constructed
group view. This is useful when you have a known set of members and want to initialize
them all with the same view of the group.

When to use
-----------

Use the "view" bootstrap method when:

- You have a predetermined list of group members
- You want to programmatically construct a group view
- You're initializing multiple processes with a shared view
- You need fine-grained control over the initial group membership

Group view structure
--------------------

A group view contains:

- A list of members, each with an address and provider ID
- Optional metadata (key-value pairs)
- A digest for version tracking

Configuration
-------------

In Bedrock configuration, you can't directly specify the view bootstrap method
because views must be constructed programmatically. Instead, you would typically:

1. Create the group using "self" bootstrap
2. Construct the desired view
3. Share the view with other processes (e.g., via file)
4. Have other processes use the "file" or "join" bootstrap methods

In C code
---------

To use the view bootstrap method programmatically:

.. code-block:: c

   #include <flock/flock-server.h>
   #include <flock/flock-group-view.h>

   // ... margo initialization ...

   struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
   flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;

   // Construct the view by adding members
   flock_group_view_add_member(&initial_view, "na+sm://12345-0", 42);
   flock_group_view_add_member(&initial_view, "na+sm://12345-1", 42);
   flock_group_view_add_member(&initial_view, "na+sm://12345-2", 42);

   // Optionally add metadata
   flock_group_view_add_metadata(&initial_view, "service", "my_service");
   flock_group_view_add_metadata(&initial_view, "version", "1.0");

   args.initial_view = &initial_view;

   const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";
   flock_provider_register(mid, provider_id, config, &args, FLOCK_PROVIDER_IGNORE);

   // Clean up the view when done
   flock_group_view_clear(&initial_view);

Sharing views between processes
--------------------------------

Once you have a group view, you can:

1. **Serialize to JSON**: Convert the view to JSON format for easy sharing
2. **Write to file**: Save the view to a file that other processes can read
3. **Send via RPC**: Transmit the view to other processes using Mercury RPC

Example of writing a view to a file:

.. code-block:: c

   #include <flock/flock-group-view.h>

   flock_group_view_t view = /* ... */;

   // Serialize to JSON string
   char* json_str = NULL;
   flock_group_view_serialize(&view, &json_str);

   // Write to file
   FILE* f = fopen("group.flock", "w");
   fprintf(f, "%s", json_str);
   fclose(f);

   free(json_str);

Other processes can then use the "file" bootstrap method to initialize from this view.

View operations
---------------

The group view API provides several operations:

- :code:`flock_group_view_add_member`: Add a member to the view
- :code:`flock_group_view_add_metadata`: Add metadata key-value pair
- :code:`flock_group_view_serialize`: Convert view to JSON string
- :code:`flock_group_view_deserialize`: Parse view from JSON string
- :code:`flock_group_view_clear`: Free view resources
- :code:`flock_group_view_copy`: Create a copy of a view

Next steps
----------

- :doc:`04_bootstrap_mpi`: Learn about MPI-based bootstrapping
- :doc:`05_bootstrap_join`: Learn about joining an existing group
- :doc:`06_bootstrap_file`: Learn about loading views from files
- :doc:`10_group_view`: Detailed guide to working with group views
