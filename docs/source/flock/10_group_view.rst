Working with group views
========================

Group views are the core data structure in Flock, representing the current
membership and metadata of a group. This tutorial covers how to create,
manipulate, and use group views.

What is a group view?
---------------------

A group view contains:

**Members**: A list of group members, each with:

- Network address (string)
- Provider ID (uint16_t)

**Metadata**: Optional key-value pairs for application-specific information

**Digest**: A version identifier that changes when the view is modified

Example: Working with a group view
----------------------------------

Here's a complete example showing how to retrieve and work with a group view:

.. literalinclude:: ../../../code/flock/10_group_view/client.c
   :language: c

Initialization
--------------

Always initialize views before use:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;

Or:

.. code-block:: c

   flock_group_view_t view;
   flock_group_view_init(&view);

Adding members
--------------

**Add a single member**:

.. code-block:: c

   flock_group_view_add_member(&view, "na+sm://12345-0", 42);
   flock_group_view_add_member(&view, "na+sm://12345-1", 42);

**From Margo instance** (self):

.. code-block:: c

   flock_group_view_init_from_self(mid, provider_id, &view);

**From MPI**:

.. code-block:: c

   flock_group_view_init_from_mpi(mid, provider_id, MPI_COMM_WORLD, &view);

Adding metadata
---------------

Metadata is optional but can be useful for:

- Service identification
- Version information
- Configuration data
- Custom application data

**Add metadata**:

.. code-block:: c

   flock_group_view_add_metadata(&view, "service", "my_service");
   flock_group_view_add_metadata(&view, "version", "1.0.0");
   flock_group_view_add_metadata(&view, "region", "us-west");

**Access metadata**:

.. code-block:: c

   for(size_t i = 0; i < view.metadata.size; i++) {
       printf("%s = %s\n",
              view.metadata.data[i].key,
              view.metadata.data[i].value);
   }

**Find metadata by key**:

.. code-block:: c

   const char* value = flock_group_view_find_metadata(&view, "version");
   if(value) {
       printf("Version: %s\n", value);
   }

File I/O
--------

**Load from file**:

.. code-block:: c

   flock_group_view_init_from_file("mygroup.flock", &view);

Cleaning up
-----------

**Free view resources**:

.. code-block:: c

   flock_group_view_clear(&view);

This frees all internal allocations (member addresses, metadata, etc.).
Always call this when done with a view.

View digest
-----------

The digest is a hash of the view contents, useful for:

- Quick comparison
- Version tracking
- Change detection

The digest is automatically updated when you modify the view through the API.

Best practices
--------------

**1. Always initialize**:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;

**2. Always clean up**:

.. code-block:: c

   flock_group_view_clear(&view);

**3. Use the API, don't manipulate directly**:

.. code-block:: c

   // Good
   flock_group_view_add_member(&view, address, provider_id);

   // Bad
   view.members.data[i].address = strdup(address);  // Don't do this!

**4. Check return values**:

.. code-block:: c

   flock_return_t ret = flock_group_view_add_member(&view, address, provider_id);
   if(ret != FLOCK_SUCCESS) {
       // Handle error
   }

Next steps
----------

- :doc:`11_bedrock`: Using Flock with Bedrock configuration
- :doc:`12_cpp`: C++ API for working with views
- :doc:`03_bootstrap_view`: Bootstrap from programmatically created views
