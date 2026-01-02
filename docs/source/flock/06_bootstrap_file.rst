Bootstrap method: file
=======================

The "file" bootstrap method allows you to initialize a Flock group by loading
a group view from a file. This is useful for persisting group membership across
restarts or sharing a group view among multiple processes.

When to use
-----------

Use the "file" bootstrap method when:

- You want to persist group membership to disk
- You're restarting a service and want to restore the previous group
- You have a pre-generated group view file to distribute
- You want to bootstrap multiple processes with the same view

File format
-----------

Flock group view files use JSON format and typically have a `.flock` extension.
A group view file contains:

- List of members (addresses and provider IDs)
- Metadata (key-value pairs)
- Digest (version information)

Example group view file:

.. code-block:: json

   {
       "members": [
           {
               "address": "na+sm://12345-0",
               "provider_id": 42
           },
           {
               "address": "na+sm://12345-1",
               "provider_id": 42
           }
       ],
       "metadata": {
           "service": "my_service",
           "version": "1.0"
       },
       "digest": 12345678
   }

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
                   "bootstrap": "file",
                   "file": "mygroup.flock",
                   "group": {
                       "type": "static",
                       "config": {}
                   }
               }
           }
       ]
   }

The "file" field specifies the path to the group view file. This can be an absolute
path or a relative path from the working directory.

In C code
---------

To load a group view from a file programmatically:

.. code-block:: c

   #include <flock/flock-server.h>
   #include <flock/flock-group-view.h>

   // ... margo initialization ...

   struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
   flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
   args.initial_view = &initial_view;

   // Load view from file
   const char* filename = "mygroup.flock";
   flock_group_view_init_from_file(filename, &initial_view);

   const char* config = "{ \"group\":{ \"type\":\"static\", \"config\":{} } }";
   flock_provider_register(mid, provider_id, config, &args, FLOCK_PROVIDER_IGNORE);

   // Clean up
   flock_group_view_clear(&initial_view);

The :code:`flock_group_view_init_from_file` function takes:

- The path to the file
- A pointer to the group view to initialize

This function reads the file, parses the JSON, and populates the group view structure.

Creating group view files
--------------------------

You can create group view files in several ways:

**1. From an existing provider**

Configure a provider to write its view to a file:

.. code-block:: json

   {
       "config": {
           "bootstrap": "self",
           "file": "mygroup.flock",
           "group": {
               "type": "static",
               "config": {}
           }
       }
   }

The provider will write its view to the specified file at initialization.

**2. Programmatically**

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;

   // Add members
   flock_group_view_add_member(&view, "na+sm://12345-0", 42);
   flock_group_view_add_member(&view, "na+sm://12345-1", 42);

   // Add metadata
   flock_group_view_add_metadata(&view, "service", "my_service");

   // Serialize to JSON
   char* json_str = NULL;
   flock_group_view_serialize(&view, &json_str);

   // Write to file
   FILE* f = fopen("mygroup.flock", "w");
   fprintf(f, "%s", json_str);
   fclose(f);

   free(json_str);
   flock_group_view_clear(&view);

**3. Manually**

You can also create the JSON file manually using a text editor, following the
format shown above.

Example workflow
----------------

A common workflow is to:

1. Start an initial member with "self" or "mpi" bootstrap
2. Configure it to write the view to a file
3. Distribute the file to other processes
4. Have those processes use "file" bootstrap to join

Initial member configuration:

.. code-block:: json

   {
       "config": {
           "bootstrap": "mpi",
           "file": "mygroup.flock",
           "group": {
               "type": "static",
               "config": {}
           }
       }
   }

Additional member configuration:

.. code-block:: json

   {
       "config": {
           "bootstrap": "file",
           "file": "mygroup.flock",
           "group": {
               "type": "static",
               "config": {}
           }
       }
   }

File updates
------------

When using a static backend, the group view file is typically written once at
initialization and not updated. With a centralized backend, you can configure
periodic updates to keep the file synchronized with the current group membership:

.. code-block:: json

   {
       "config": {
           "bootstrap": "file",
           "file": "mygroup.flock",
           "update_file": true,
           "group": {
               "type": "centralized",
               "config": {}
           }
       }
   }

When :code:`update_file` is true, the provider will periodically rewrite the file
with the current group view.

Error handling
--------------

File bootstrap can fail if:

- The file doesn't exist or isn't readable
- The JSON is malformed
- The file format is incorrect
- Member addresses are invalid

Always check for errors:

.. code-block:: c

   flock_return_t ret = flock_group_view_init_from_file(filename, &initial_view);

   if (ret != FLOCK_SUCCESS) {
       fprintf(stderr, "Failed to load group view from %s: error %d\n",
               filename, ret);
       // Handle error
   }

Next steps
----------

- :doc:`07_backends_static`: Learn about the static backend
- :doc:`08_backends_centralized`: Learn about the centralized backend
- :doc:`10_group_view`: Detailed guide to working with group views
