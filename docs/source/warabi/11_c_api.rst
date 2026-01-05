C API
=====

While Warabi is primarily a C++ library, it also provides a C API for use with
Margo-based applications. This tutorial covers the C API, which is useful when
you need to integrate Warabi with C code or prefer a C-style interface.

Headers and types
-----------------

.. code-block:: c

   #include <warabi/client.h>   // Client-side
   #include <warabi/server.h>   // Server-side
   #include <warabi/error.h>    // Error handling

Core types
----------

**Client and handles**:

.. code-block:: c

   warabi_client_t           // Client instance
   warabi_target_handle_t    // Handle to a remote target
   warabi_async_request_t    // Asynchronous request handle

**Region ID**:

.. code-block:: c

   typedef struct warabi_region {
       uint8_t opaque[16];
   } warabi_region_t;

**Error codes**:

.. code-block:: c

   warabi_err_t              // Error code (check with != WARABI_SUCCESS)

   #define WARABI_SUCCESS       0
   #define WARABI_ERR_INVALID_ARGS
   #define WARABI_ERR_ALLOCATION
   #define WARABI_ERR_NOT_FOUND
   // ... more error codes

Server-side (Provider)
----------------------

**Creating a provider**:

.. code-block:: c

   #include <margo.h>
   #include <warabi/server.h>

   margo_instance_id mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);

   // Warabi configuration (JSON)
   const char* config = "{"
       "\"targets\": ["
           "{\"type\": \"memory\", \"config\": {}}"
       "]"
   "}";

   warabi_provider_t provider;
   warabi_err_t ret = warabi_provider_register(
       mid,                    // Margo instance
       42,                     // Provider ID
       config,                 // Configuration
       NULL,                   // Default pool
       &provider               // Output provider
   );

   if(ret != WARABI_SUCCESS) {
       fprintf(stderr, "Failed to create provider\\n");
       return -1;
   }

   margo_wait_for_finalize(mid);

**Cleanup**:

.. code-block:: c

   warabi_provider_destroy(provider);
   margo_finalize(mid);

Client initialization
---------------------

**Creating a client**:

.. code-block:: c

   #include <margo.h>
   #include <warabi/client.h>

   margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);

   warabi_client_t client;
   warabi_err_t ret = warabi_client_create(mid, &client);
   if(ret != WARABI_SUCCESS) {
       fprintf(stderr, "Failed to create client\\n");
       return -1;
   }

**Creating target handles**:

.. code-block:: c

   const char* server_addr = "na+sm://12345-0";
   uint16_t provider_id = 42;

   warabi_target_handle_t target;
   ret = warabi_client_make_target_handle(
       client,
       server_addr,
       provider_id,
       &target
   );

   if(ret != WARABI_SUCCESS) {
       fprintf(stderr, "Failed to create target handle\\n");
       return -1;
   }

**Cleanup**:

.. code-block:: c

   warabi_target_handle_free(target);
   warabi_client_free(client);
   margo_finalize(mid);

Region operations
-----------------

**Creating a region**:

.. code-block:: c

   warabi_region_t region;
   size_t size = 1024;  // 1 KB

   ret = warabi_create(
       target,                      // Target handle
       size,                        // Region size
       &region,                     // Output region ID
       WARABI_ASYNC_REQUEST_IGNORE // Synchronous (no async)
   );

   if(ret != WARABI_SUCCESS) {
       fprintf(stderr, "Failed to create region\\n");
       return -1;
   }

**Writing data**:

.. code-block:: c

   const char* data = "Hello, Warabi!";
   size_t data_size = strlen(data);
   size_t offset = 0;
   bool persist = false;

   ret = warabi_write(
       target,
       region,
       offset,
       data,
       data_size,
       persist,
       WARABI_ASYNC_REQUEST_IGNORE
   );

**Reading data**:

.. code-block:: c

   char buffer[1024];
   size_t buffer_size = sizeof(buffer);
   size_t offset = 0;

   ret = warabi_read(
       target,
       region,
       offset,
       buffer,
       buffer_size,
       WARABI_ASYNC_REQUEST_IGNORE
   );

   if(ret == WARABI_SUCCESS) {
       printf("Read: %.*s\\n", (int)buffer_size, buffer);
   }

**Erasing a region**:

.. code-block:: c

   ret = warabi_erase(
       target,
       region,
       WARABI_ASYNC_REQUEST_IGNORE
   );

Asynchronous operations
-----------------------

**Async write**:

.. code-block:: c

   warabi_async_request_t req;

   ret = warabi_write(
       target,
       region,
       0,        // offset
       data,
       size,
       false,    // persist
       &req      // Async request (not IGNORE)
   );

   // Do other work...

   // Wait for completion
   ret = warabi_wait(req);

**Async read**:

.. code-block:: c

   char buffer[1024];
   warabi_async_request_t req;

   ret = warabi_read(
       target,
       region,
       0,
       buffer,
       sizeof(buffer),
       &req
   );

   // Do other work...

   ret = warabi_wait(req);

**Testing async completion**:

.. code-block:: c

   bool completed = false;

   while(!completed) {
       ret = warabi_test(req, &completed);
       if(completed) {
           printf("Operation completed!\\n");
       } else {
           // Do other work...
       }
   }

Error handling
--------------

**Checking errors**:

.. code-block:: c

   warabi_err_t ret = warabi_create(target, size, &region, NULL);

   if(ret != WARABI_SUCCESS) {
       fprintf(stderr, "Error: %s\\n", warabi_error_string(ret));
       // Handle error
   }

**Common error codes**:

.. code-block:: c

   WARABI_SUCCESS          // Success
   WARABI_ERR_INVALID_ARGS // Invalid argument
   WARABI_ERR_ALLOCATION   // Memory allocation failed
   WARABI_ERR_NOT_FOUND    // Region not found
   WARABI_ERR_BACKEND      // Backend-specific error
   WARABI_ERR_RPC          // RPC/network error

Complete example
----------------

Full C client application:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c

This example demonstrates:

- Initializing Margo and Warabi client
- Looking up server address
- Creating target handle
- Creating a region
- Writing and reading data
- Proper error checking
- Cleanup

Compiling and running
---------------------

**With CMake**:

.. literalinclude:: ../../../code/warabi/11_c_api/CMakeLists.txt
   :language: cmake

**With pkg-config**:

.. code-block:: console

   $ gcc -o client client.c \\
       $(pkg-config --cflags --libs warabi-client margo)

   $ ./client na+sm://12345-0 42

**Expected output**:

.. code-block:: console

   Client initialized
   Connected to target
   Created region
   Wrote 14 bytes
   Read: Hello, Warabi!
   SUCCESS: Data verified
   Region erased

Comparing C and C++ APIs
-------------------------

**C API characteristics**:

- Explicit error handling with return codes
- Manual resource management (must call free functions)
- Compatible with C codebases
- Works with Margo (not Thallium)
- Requires careful memory management

**C++ API characteristics**:

- Exception-based error handling
- RAII for automatic resource management
- Modern C++ features (move semantics, etc.)
- Works with Thallium
- Cleaner, more concise code

**When to use C API**:

- Integrating with existing C code
- Building Margo-based applications
- Need explicit control over resources
- Prefer return code error handling

**When to use C++ API**:

- New C++ applications
- Using Thallium
- Want RAII and modern C++ features
- Prefer exception-based error handling

Common patterns
---------------

**Checking if region exists**:

.. code-block:: c

   // Try to read - will fail if region doesn't exist
   char test_buffer[1];
   ret = warabi_read(target, region, 0, test_buffer, 1, NULL);

   if(ret == WARABI_ERR_NOT_FOUND) {
       printf("Region doesn't exist\\n");
   }

**Multiple regions**:

.. code-block:: c

   #define NUM_REGIONS 3
   warabi_region_t regions[NUM_REGIONS];

   for(int i = 0; i < NUM_REGIONS; i++) {
       ret = warabi_create(target, 1024, &regions[i], NULL);
       if(ret != WARABI_SUCCESS) {
           // Handle error
       }
   }

   // Use regions...

   for(int i = 0; i < NUM_REGIONS; i++) {
       warabi_erase(target, regions[i], NULL);
   }

**Bulk data transfer**:

.. code-block:: c

   // For large data, use bulk transfer
   size_t large_size = 1024 * 1024;  // 1 MB
   char* large_data = malloc(large_size);

   hg_bulk_t bulk;
   // Create bulk handle (Mercury API)
   // ...

   ret = warabi_write_bulk(
       target,
       region,
       0,           // offset
       bulk,        // bulk handle
       0,           // bulk offset
       large_size,
       false,       // persist
       NULL
   );

   free(large_data);

Best practices
--------------

**1. Always check return codes**:

.. code-block:: c

   warabi_err_t ret = warabi_create(target, size, &region, NULL);
   if(ret != WARABI_SUCCESS) {
       fprintf(stderr, "Error: %s\\n", warabi_error_string(ret));
       goto cleanup;
   }

**2. Use cleanup labels**:

.. code-block:: c

   warabi_target_handle_t target = WARABI_TARGET_HANDLE_NULL;
   warabi_client_t client = WARABI_CLIENT_NULL;

   // ... operations ...

cleanup:
   if(target != WARABI_TARGET_HANDLE_NULL)
       warabi_target_handle_free(target);
   if(client != WARABI_CLIENT_NULL)
       warabi_client_free(client);

**3. Initialize pointers to NULL**:

.. code-block:: c

   warabi_client_t client = WARABI_CLIENT_NULL;
   warabi_target_handle_t target = WARABI_TARGET_HANDLE_NULL;

**4. Free in reverse order of creation**:

.. code-block:: c

   // Create order: client, target
   // Free order: target, client
   warabi_target_handle_free(target);
   warabi_client_free(client);

**5. Use async for performance**:

For better performance with multiple operations, use asynchronous calls:

.. code-block:: c

   warabi_async_request_t reqs[10];

   // Issue multiple async writes
   for(int i = 0; i < 10; i++) {
       warabi_write(target, regions[i], 0, data[i], sizes[i],
                    false, &reqs[i]);
   }

   // Wait for all
   for(int i = 0; i < 10; i++) {
       warabi_wait(reqs[i]);
   }

Next steps
----------

- :doc:`01_intro`: Warabi basics (in C++)
- :doc:`09_async`: Advanced async patterns
- :ref:`Margo`: Learn about Margo (for C API)
- :ref:`Thallium`: Learn about Thallium (for C++ API)
