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

.. code-block:: c

   #include <stdio.h>
   #include <stdlib.h>
   #include <margo.h>
   #include <flock/flock-client.h>
   #include <flock/flock-group.h>

   int main(int argc, char** argv)
   {
       if(argc != 3) {
           fprintf(stderr, "Usage: %s <server address> <provider id>\n", argv[0]);
           exit(-1);
       }

       const char* svr_addr_str = argv[1];
       uint16_t provider_id = atoi(argv[2]);

       // Initialize Margo in client mode
       margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);

       // Lookup server address
       hg_addr_t svr_addr;
       margo_addr_lookup(mid, svr_addr_str, &svr_addr);

       // Create Flock client
       flock_client_t client;
       flock_client_init(mid, ABT_POOL_NULL, &client);

       // Create group handle
       flock_group_handle_t group_handle;
       flock_group_handle_create(client, svr_addr, provider_id, true, &group_handle);

       // Query group membership
       flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
       flock_group_handle_get_view(group_handle, &view);

       printf("Group has %zu members:\n", view.members.count);
       for(size_t i = 0; i < view.members.count; i++) {
           printf("  [%zu] %s (provider_id=%d)\n",
                  i, view.members.data[i].address,
                  view.members.data[i].provider_id);
       }

       // Clean up
       flock_group_view_clear(&view);
       flock_group_handle_release(group_handle);
       flock_client_finalize(client);
       margo_addr_free(mid, svr_addr);
       margo_finalize(mid);

       return 0;
   }

Client initialization
---------------------

**Create a client**:

.. code-block:: c

   flock_client_t client;
   flock_return_t ret = flock_client_init(mid, pool, &client);

Parameters:
- :code:`mid`: Margo instance
- :code:`pool`: Argobots pool for RPCs (or :code:`ABT_POOL_NULL` for default)
- :code:`client`: Pointer to store the client handle

**Finalize the client**:

.. code-block:: c

   flock_client_finalize(client);

This cleans up resources associated with the client.

Group handle operations
-----------------------

**Create a group handle**:

.. code-block:: c

   flock_group_handle_t group_handle;
   flock_return_t ret = flock_group_handle_create(
       client, svr_addr, provider_id, refresh, &group_handle);

Parameters:
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

.. code-block:: c

   flock_group_handle_release(group_handle);

Querying group membership
--------------------------

**Get the full view**:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_return_t ret = flock_group_handle_get_view(group_handle, &view);

   // Use the view
   printf("Group size: %zu\n", view.members.count);

   // Clean up
   flock_group_view_clear(&view);

This creates a **copy** of the current group view. You own this copy and must
call :code:`flock_group_handle_get_view` returns a snapshot of the membership at the time of the call.

**Access view without copying**:

For read-only access without copying:

.. code-block:: c

   const flock_group_view_t* view;
   flock_return_t ret = flock_group_handle_access_view(
       group_handle, &view);

   // Use the view (read-only)
   printf("Group size: %zu\n", view->members.count);

   // Release access
   flock_group_handle_release_view(group_handle);

This gives you a pointer to the handle's internal view. The view remains valid
until you call :code:`flock_group_handle_release_view`.

Accessing members
-----------------

**Get member by rank**:

.. code-block:: c

   const char* address;
   uint16_t provider_id;
   flock_return_t ret = flock_group_handle_get_member(
       group_handle, rank, &address, &provider_id);

   printf("Member %zu: %s (provider_id=%d)\n", rank, address, provider_id);

**Get group size**:

.. code-block:: c

   size_t size;
   flock_group_handle_get_size(group_handle, &size);
   printf("Group has %zu members\n", size);

Metadata access
---------------

If the group has metadata, you can access it:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_group_handle_get_view(group_handle, &view);

   // Check if metadata exists
   for(size_t i = 0; i < view.metadata.count; i++) {
       printf("  %s = %s\n",
              view.metadata.data[i].key,
              view.metadata.data[i].value);
   }

   flock_group_view_clear(&view);

Membership change callbacks
----------------------------

For dynamic groups, you can register callbacks to be notified of changes:

.. code-block:: c

   void membership_changed(void* context, const flock_group_view_t* view) {
       printf("Group membership changed! New size: %zu\n", view->members.count);
   }

   // Register callback
   flock_group_handle_register_membership_callback(
       group_handle, membership_changed, NULL);

   // Unregister callback
   flock_group_handle_deregister_membership_callback(
       group_handle, membership_changed);

The callback will be invoked whenever the group membership changes (for
centralized backend only).

Example: Service discovery
---------------------------

A common use case is discovering service instances:

.. code-block:: c

   // Connect to any known member
   flock_group_handle_create(client, bootstrap_addr, provider_id,
                              true, &group_handle);

   // Get full membership
   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_group_handle_get_view(group_handle, &view);

   // Connect to all members
   for(size_t i = 0; i < view.members.count; i++) {
       hg_addr_t member_addr;
       margo_addr_lookup(mid, view.members.data[i].address, &member_addr);

       // Create service handle to this member
       // ... (service-specific code)

       margo_addr_free(mid, member_addr);
   }

   flock_group_view_clear(&view);

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

Thread safety
-------------

The Flock client API is thread-safe when used with proper Argobots pools:

.. code-block:: c

   // Create a pool for Flock operations
   ABT_pool flock_pool;
   ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                          ABT_TRUE, &flock_pool);

   // Initialize client with this pool
   flock_client_init(mid, flock_pool, &client);

This allows multiple threads/ULTs to use the same client safely.

Next steps
----------

- :doc:`10_group_view`: Detailed guide to working with group views
- :doc:`11_bedrock`: Using Flock with Bedrock
- :doc:`12_cpp`: C++ API for Flock
