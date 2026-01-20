Bootstrap method: file
=======================

The "file" bootstrap method allows you to initialize a Flock group by loading
a group view from a file. This assumes that a JSON file with the Flock format
has been created beforehand.

When to use
-----------

Use the "file" bootstrap method when:

- You have a pre-generated group view file to distribute
- You're restarting a service and want to restore the previous group
- You want to bootstrap multiple processes with the same view

File format
-----------

Flock group view files use JSON format and typically have a `.flock` extension.
A group view file contains:

- List of members (addresses and provider IDs)
- Metadata (key-value pairs)
- Digest (version information)

Example group view file:

.. literalinclude:: ../../../code/flock/06_bootstrap_file/view.json
   :language: json

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

.. literalinclude:: ../../../code/flock/06_bootstrap_file/server.c
   :language: c

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

**2. Manually**

You can create the JSON file manually using a text editor, following the
format shown above.
