C API
=====

While Warabi is primarily a C++ library, it also provides a C API for use with
Margo-based applications. This tutorial covers the C API, which is useful when
you need to integrate Warabi with C code or prefer a C-style interface.

Headers and types
-----------------

Include the following headers:

.. code-block:: c

   #include <warabi/client.h> // if using client library
   #include <warabi/server.h> // if using server library

Core types:

- ``warabi_client_t``: Client instance
- ``warabi_provider_t``: Provider instance
- ``warabi_target_handle_t``: Handle to a remote target
- ``warabi_region_t``: Region identifier (16-byte opaque structure)
- ``warabi_async_request_t``: Asynchronous request handle
- ``warabi_err_t``: Error code (check with ``!= WARABI_SUCCESS``)

Server-side (Provider)
----------------------

Creating a provider requires initializing Margo and registering a Warabi provider
with a configuration:

.. literalinclude:: ../../../code/warabi/11_c_api/server.c
   :language: c
   :lines: 18-39

Key points:

- Configuration is provided as a JSON string
- Use ``"target": {}`` (singular, not ``"targets": []``)
- Provider ID should match what clients will use
- Call ``margo_wait_for_finalize()`` to keep server running

Client initialization
---------------------

**Creating a client**:

Initialize Margo in client mode and create a Warabi client:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 28-41

**Creating target handles**:

To access a target, create a handle with the server address and provider ID:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 50-61

**Cleanup**:

Always free resources in reverse order of creation:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 131-139

Region operations
-----------------

**Creating a region**:

Regions must be created with a fixed size:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 63-77

**Writing data**:

Write data to a region at a specific offset:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 79-96

The ``persist`` parameter controls whether data is immediately flushed to durable
storage (for ``pmem`` and ``abtio`` backends).

**Reading data**:

Read data back from a region:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 98-114

**Erasing a region**:

Clean up regions when done:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c
   :lines: 123-128

Asynchronous operations
-----------------------

For better performance, use asynchronous operations that don't block:

**Async write**:

Issue a write operation that returns immediately:

.. literalinclude:: ../../../code/warabi/11_c_api/async_example.c
   :language: c
   :lines: 57-75

The operation continues in the background. You can do other work while it completes.

**Waiting for completion**:

Use ``warabi_wait()`` to wait for an async operation to complete:

.. literalinclude:: ../../../code/warabi/11_c_api/async_example.c
   :language: c
   :lines: 77-86

**Testing for completion**:

Use ``warabi_test()`` to check if an operation has completed without blocking:

.. literalinclude:: ../../../code/warabi/11_c_api/async_example.c
   :language: c
   :lines: 107-119

Complete examples
-----------------

**Client example**:

Full working client that creates a region, writes data, reads it back, and verifies:

.. literalinclude:: ../../../code/warabi/11_c_api/client.c
   :language: c

**Server example**:

Warabi server that exposes a memory-backed target:

.. literalinclude:: ../../../code/warabi/11_c_api/server.c
   :language: c

**Async example**:

Demonstrates asynchronous write and read operations with completion testing:

.. literalinclude:: ../../../code/warabi/11_c_api/async_example.c
   :language: c
