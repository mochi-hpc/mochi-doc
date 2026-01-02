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

Group view structure
--------------------

In C, a group view is represented by :code:`flock_group_view_t`:

.. code-block:: c

   typedef struct flock_group_view {
       struct {
           size_t count;
           struct {
               char* address;
               uint16_t provider_id;
           }* data;
       } members;

       struct {
           size_t count;
           struct {
               char* key;
               char* value;
           }* data;
       } metadata;

       uint64_t digest;
   } flock_group_view_t;

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

   for(size_t i = 0; i < view.metadata.count; i++) {
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

Serialization
-------------

Views can be serialized to/from JSON for storage or transmission.

**Serialize to JSON**:

.. code-block:: c

   char* json_str = NULL;
   flock_return_t ret = flock_group_view_serialize(&view, &json_str);

   if(ret == FLOCK_SUCCESS) {
       printf("Serialized view:\n%s\n", json_str);
       free(json_str);
   }

Example output:

.. code-block:: json

   {
       "members": [
           {"address": "na+sm://12345-0", "provider_id": 42},
           {"address": "na+sm://12345-1", "provider_id": 42}
       ],
       "metadata": {
           "service": "my_service",
           "version": "1.0.0"
       },
       "digest": 1234567890
   }

**Deserialize from JSON**:

.. code-block:: c

   const char* json_str = /* ... */;
   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;

   flock_return_t ret = flock_group_view_deserialize(json_str, &view);

   if(ret == FLOCK_SUCCESS) {
       printf("Loaded view with %zu members\n", view.members.count);
       flock_group_view_clear(&view);
   }

File I/O
--------

**Save to file**:

.. code-block:: c

   flock_return_t save_view_to_file(const flock_group_view_t* view,
                                      const char* filename)
   {
       char* json_str = NULL;
       flock_return_t ret = flock_group_view_serialize(view, &json_str);
       if(ret != FLOCK_SUCCESS) return ret;

       FILE* f = fopen(filename, "w");
       if(!f) {
           free(json_str);
           return FLOCK_ERR_INVALID_ARG;
       }

       fprintf(f, "%s", json_str);
       fclose(f);
       free(json_str);

       return FLOCK_SUCCESS;
   }

**Load from file**:

.. code-block:: c

   flock_return_t load_view_from_file(const char* filename,
                                        flock_group_view_t* view)
   {
       FILE* f = fopen(filename, "r");
       if(!f) return FLOCK_ERR_INVALID_ARG;

       fseek(f, 0, SEEK_END);
       long size = ftell(f);
       fseek(f, 0, SEEK_SET);

       char* json_str = malloc(size + 1);
       fread(json_str, 1, size, f);
       json_str[size] = '\0';
       fclose(f);

       flock_return_t ret = flock_group_view_deserialize(json_str, view);
       free(json_str);

       return ret;
   }

Or use the built-in function:

.. code-block:: c

   flock_group_view_init_from_file("mygroup.flock", &view);

Copying views
-------------

**Make a copy**:

.. code-block:: c

   flock_group_view_t view1 = /* ... */;
   flock_group_view_t view2 = FLOCK_GROUP_VIEW_INITIALIZER;

   flock_group_view_copy(&view1, &view2);

   // Now view2 is an independent copy of view1

**Assignment copies**:

Note that simple assignment does NOT copy:

.. code-block:: c

   flock_group_view_t view2 = view1;  // WRONG: shallow copy

This creates a shallow copy where both views share the same internal arrays.
Use :code:`flock_group_view_copy` instead.

Comparing views
---------------

**Check equality**:

.. code-block:: c

   bool equal = flock_group_view_equals(&view1, &view2);

This compares:
- Member count and addresses
- Metadata
- Digest

**Check digest only**:

For quick comparison, check if digests match:

.. code-block:: c

   if(view1.digest == view2.digest) {
       // Views are likely identical
   }

Note: digest equality doesn't guarantee full equality, but inequality guarantees
the views differ.

Cleaning up
-----------

**Free view resources**:

.. code-block:: c

   flock_group_view_clear(&view);

This frees all internal allocations (member addresses, metadata, etc.).
Always call this when done with a view.

**Note**: Don't call :code:`free()` on the view itself if it's stack-allocated:

.. code-block:: c

   flock_group_view_t view = FLOCK_GROUP_VIEW_INITIALIZER;
   // ... use view ...
   flock_group_view_clear(&view);
   // Don't call free(&view)!

View digest
-----------

The digest is a hash of the view contents, useful for:
- Quick comparison
- Version tracking
- Change detection

**Update digest**:

The digest is automatically updated when you modify the view through the API.
If you manually modify the view structure, update the digest:

.. code-block:: c

   flock_group_view_update_digest(&view);

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

**5. Copy when needed**:

If you need to keep a view across function calls or while the source view might
change, make a copy:

.. code-block:: c

   flock_group_view_t my_copy = FLOCK_GROUP_VIEW_INITIALIZER;
   flock_group_view_copy(&original, &my_copy);

Next steps
----------

- :doc:`11_bedrock`: Using Flock with Bedrock configuration
- :doc:`12_cpp`: C++ API for working with views
- :doc:`03_bootstrap_view`: Bootstrap from programmatically created views
