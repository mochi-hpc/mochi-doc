Client API
==========

The Flock client API allows external processes to query group membership and
interact with Flock providers without being group members themselves.

Overview
--------

The client API provides:

- **Group handle creation**: Connect to a Flock provider
- **View querying**: Retrieve the current group membership
- **Member discovery**: Find members by rank or address
- **Metadata access**: Read group metadata

This is useful for:

- Load balancers that need to discover service instances
- Monitoring tools that track group membership
- Clients that need to connect to any group member

Basic client usage
------------------

Here's a complete example of using the Flock client API:

.. literalinclude:: ../../../code/flock/09_client_api/client.c
   :language: c

Client initialization
---------------------

**Create a client**:

The client is created using :code:`flock_client_init`:

- :code:`mid`: Margo instance
- :code:`pool`: Argobots pool for RPCs (or :code:`ABT_POOL_NULL` for default)
- :code:`client`: Pointer to store the client handle

**Finalize the client**:

Use :code:`flock_client_finalize(client)` to clean up resources associated with the client.

Group handle operations
-----------------------

**Create a group handle**:

Use :code:`flock_group_handle_create` with:

- :code:`client`: The Flock client
- :code:`svr_addr`: Mercury address of a group member
- :code:`provider_id`: Provider ID of that member
- :code:`refresh`: Whether to auto-refresh the view
- :code:`group_handle`: Pointer to store the handle

The :code:`refresh` parameter controls whether the handle automatically updates
its cached view. Set to :code:`true` for dynamic groups, :code:`false` for static groups.

**Create from a file**:

You can also create a group handle from a serialized view file:

.. code-block:: c

   flock_group_handle_t group_handle;
   flock_return_t ret = flock_group_handle_create_from_file(
       client, "mygroup.flock", refresh, &group_handle);

This loads the view from the file and creates handles to the members.

**Release a group handle**:

Use :code:`flock_group_handle_release(group_handle)` when done.

Querying group membership
--------------------------

**Get the full view**:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_return_t ret = flock_group_get_view(group_handle, &view);

   // Use the view
   printf("Group size: %zu\n", view.members.size);

   // Clean up
   flock_group_view_clear(&view);

This creates a **copy** of the current group view. You own this copy and must
call :code:`flock_group_view_clear` to free it.

Error handling
--------------

Always check return values:

.. code-block:: c

   flock_return_t ret = flock_client_init(mid, ABT_POOL_NULL, &client);
   if(ret != FLOCK_SUCCESS) {
       fprintf(stderr, "Failed to initialize client: %d\n", ret);
       return -1;
   }

Common error codes:

- :code:`FLOCK_SUCCESS`: Operation succeeded
- :code:`FLOCK_ERR_INVALID_ARG`: Invalid argument
- :code:`FLOCK_ERR_MERCURY`: Mercury error
- :code:`FLOCK_ERR_NOT_FOUND`: Member not found

Next steps
----------

- :doc:`10_group_view`: Detailed guide to working with group views
- :doc:`11_bedrock`: Using Flock with Bedrock
- :doc:`12_cpp`: C++ API for Flock
