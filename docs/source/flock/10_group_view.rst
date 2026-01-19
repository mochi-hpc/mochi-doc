Working with group views
========================

Group views are the core data structure in Flock, representing the current (or desired, in the case of
bootstrapping) membership and metadata of a group. This tutorial covers how to create,
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

**Initialization from a Margo instance** (self):

.. code-block:: c

   flock_group_view_init_from_self(mid, provider_id, &view);

**Initialization from MPI**:

.. code-block:: c

   flock_group_view_init_from_mpi(mid, provider_id, MPI_COMM_WORLD, &view);

Adding/removing members
-----------------------

**Add a single member**:

.. code-block:: c

   flock_group_view_add_member(&view, "na+sm://12345-0", 42);
   flock_group_view_add_member(&view, "na+sm://12345-1", 42);

This function returns the ``flock_member_t*`` pointer to the newly created
member structure in the view. This pointer can be used to e.g. manipulate
its ``extra`` field and attach data to the member. This pointer is also what
will be needed to remove a member.

**Removing a member**:

.. code-block:: c

   flock_group_view_remove_member(&view, member);

Accessing members
-----------------

**Get the number of members**:

.. code-block:: c

   size_t count = flock_group_view_member_count(&view);

**Get a member by index**:

.. code-block:: c

   flock_member_t* member = flock_group_view_member_at(&view, 0);
   if(member) {
       printf("Address: %s, Provider ID: %u\n",
              member->address, member->provider_id);
   }

**Find a member by address and provider ID**:

.. code-block:: c

   flock_member_t* member = flock_group_view_find_member(
       &view, "na+sm://12345-0", 42);
   if(member) {
       // Member found
   }

**Compare two members**:

.. code-block:: c

   int cmp = flock_member_cmp(member1, member2);
   // Returns 0 if equal, -1 if member1 < member2, 1 if member1 > member2

**Iterate over all members**:

.. code-block:: c

   for(size_t i = 0; i < flock_group_view_member_count(&view); i++) {
       flock_member_t* m = flock_group_view_member_at(&view, i);
       printf("Member %zu: %s (provider %u)\n",
              i, m->address, m->provider_id);
   }

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

**Remove metadata by key**:

.. code-block:: c

   bool removed = flock_group_view_remove_metadata(&view, "region");
   // Returns true if metadata was removed, false if key wasn't found

**Get the number of metadata entries**:

.. code-block:: c

   size_t count = flock_group_view_metadata_count(&view);

**Get metadata by index**:

.. code-block:: c

   flock_metadata_t* meta = flock_group_view_metadata_at(&view, 0);
   if(meta) {
       printf("Key: %s, Value: %s\n", meta->key, meta->value);
   }

**Iterate over all metadata**:

.. code-block:: c

   for(size_t i = 0; i < flock_group_view_metadata_count(&view); i++) {
       flock_metadata_t* m = flock_group_view_metadata_at(&view, i);
       printf("%s = %s\n", m->key, m->value);
   }

Serialization
-------------

**Serialize to a file**:

.. code-block:: c

   flock_return_t ret = flock_group_view_serialize_to_file(&view, "mygroup.flock");
   if(ret != FLOCK_SUCCESS) {
       // Handle error
   }

**Deserialize from a file**:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_return_t ret = flock_group_view_from_file("mygroup.flock", &view);
   if(ret != FLOCK_SUCCESS) {
       // Handle error
   }

**Deserialize from a string**:

.. code-block:: c

   const char* json_str = "{...}";  // JSON representation of the view
   flock_return_t ret = flock_group_view_from_string(json_str, strlen(json_str), &view);

**Serialize with a custom callback**:

.. code-block:: c

   void my_serializer(void* ctx, const char* data, size_t len) {
       // Custom serialization logic, e.g. write to a buffer
   }

   flock_group_view_serialize(&view, my_serializer, my_context);

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

**Get the digest**:

.. code-block:: c

   uint64_t digest = flock_group_view_digest(&view);

The digest is automatically updated when you modify the view through the API.

Moving views
------------

The ``FLOCK_GROUP_VIEW_MOVE`` macro efficiently moves the content of one view
into another without copying data:

.. code-block:: c

   flock_group_view_t src = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_group_view_t dst = FLOCK_GROUP_VIEW_INITIALIZER;

   // ... populate src ...

   FLOCK_GROUP_VIEW_MOVE(&src, &dst);
   // src is now empty, dst contains the data

**Warning**: The destination must be empty (or already cleared) before the move,
otherwise it will cause a memory leak.

Thread safety
-------------

Group views include a mutex for thread-safe access. Use the provided macros
when accessing or modifying views from multiple threads:

.. code-block:: c

   FLOCK_GROUP_VIEW_LOCK(&view);
   // Access or modify the view safely
   size_t count = flock_group_view_member_count(&view);
   FLOCK_GROUP_VIEW_UNLOCK(&view);

Note: The individual functions (``flock_group_view_add_member``, etc.) do not
acquire the lock themselves. It is the caller's responsibility to lock the view
when concurrent access is possible.

Extra data
----------

Each member has an ``extra`` field that backends can use to attach custom data:

.. code-block:: c

   flock_member_t* member = flock_group_view_add_member(&view, address, id);
   member->extra.data = my_custom_data;
   member->extra.free = my_free_function;  // Called when member is removed

**Clear all extra data**:

.. code-block:: c

   flock_group_view_clear_extra(&view);

This calls the free function for each member's extra data and sets the pointers
to NULL, but does not remove the members from the view.

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

   // Functions returning pointers: check for NULL
   flock_member_t* member = flock_group_view_add_member(&view, address, provider_id);
   if(!member) {
       // Handle allocation error
   }

   // Functions returning flock_return_t: check for FLOCK_SUCCESS
   flock_return_t ret = flock_group_view_serialize_to_file(&view, "mygroup.flock");
   if(ret != FLOCK_SUCCESS) {
       // Handle error
   }

**5. Lock when accessing from multiple threads**:

.. code-block:: c

   FLOCK_GROUP_VIEW_LOCK(&view);
   // ... access or modify ...
   FLOCK_GROUP_VIEW_UNLOCK(&view);
