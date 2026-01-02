Getting started with Warabi
===========================

Installing Warabi
-----------------

Warabi can be installed using Spack as follows.

.. code-block:: console

   spack install mochi-warabi +bedrock

The :code:`+bedrock` variant will enable :ref:`Bedrock` support, which will be useful
for spinning up a Warabi provider without having to write code.
The :code:`spack info mochi-warabi` command can be used to show the list of variants
available.

In the following sections, the code can be compiled and linked against the *warabi-server*
and *warabi-client* libraries, which can be found either by calling
:code:`find_package(warabi)` in CMake, or :code:`pkg-config --libs --cflags warabi-server`
(respectively :code:`warabi-client`) with PkgConfig.

What is Warabi?
---------------

Warabi is a blob storage microservice for Mochi. It provides capabilities for:

- **Blob Storage**: Storing and retrieving binary large objects (BLOBs)
- **Multiple Backends**: Memory, persistent memory (pmem), ABT-IO for async I/O
- **Region Management**: Organizing blobs into logical regions
- **Bulk Transfers**: Efficient RDMA-based data transfer
- **Data Migration**: Moving data between providers or storage backends
- **Async Operations**: Non-blocking I/O for high performance

Warabi is particularly useful for:
- Storing checkpoint data in HPC applications
- Caching large datasets
- Managing temporary storage for workflows
- Implementing distributed object stores

Key concepts
------------

**Targets**: A target is a storage endpoint that can hold regions. Each provider
can manage multiple targets, each potentially using a different backend.

**Regions**: A region is a logical container for data. You create regions,
write data to them, and read data from them. Each region has a unique ID.

**Transfer Managers**: Control how data is transferred between client and server,
with options for pipelining and concurrency.

**Backends**: The underlying storage implementation (memory, pmem, abt-io, dummy).

Instantiating a Warabi provider
--------------------------------

Warabi adopts the typical Mochi microservice architecture
(used for instance in the :ref:`Thallium microservice template<thallium-microservice-template>`),
with a *server* library providing the microservice's *provider* implementation,
and a *client* library providing access to its capabilities (e.g., creating and
accessing regions).

Since we have enabled Bedrock support, let's take advantage of that and write
a *bedrock-config.json* file for Bedrock to use (if you are not familiar with Bedrock,
I highly recommend you to read the :ref:`Bedrock` section. Using Bedrock will save
you development time since it allows you to bootstrap a Mochi service using a JSON file
instead of writing code).

.. code-block:: json

   {
       "libraries": {
           "warabi": "libwarabi-bedrock-module.so"
       },
       "providers": [
           {
               "type": "warabi",
               "name": "my_warabi_provider",
               "provider_id": 42,
               "config": {
                   "targets": [
                       {
                           "type": "memory",
                           "config": {}
                       }
                   ]
               }
           }
       ]
   }

We can now give this config file to Bedrock as follows.

.. code-block:: console

   $ bedrock na+sm -c bedrock-config.json
   [info] [warabi] Warabi provider started
   [info] Bedrock daemon now running at na+sm://12345-0

We now have a Warabi provider running, with a *provider id* of 42, managing
one target using the "memory" backend.

If you need to create a provider in C++ (either because you don't want to use Bedrock
or because you want your provider to be embedded into an existing application), the
following code shows how to do that.

.. literalinclude:: ../../../code/warabi/01_intro/server.cpp
   :language: cpp

The key steps in this code are:

1. Initialize Thallium engine in server mode
2. Create a JSON configuration specifying the targets and backends
3. Create the Warabi provider with the engine, provider ID, and configuration

Interacting with targets via the client interface
--------------------------------------------------

Now we can use the client library to create a client object,
create a target handle, and start interacting with our storage.

.. literalinclude:: ../../../code/warabi/01_intro/client.cpp
   :language: cpp

The client is created using :code:`warabi::Client`.
We then create a :code:`warabi::TargetHandle`. This handle
is the object that will let us interact with the target.

The target handle is created using :code:`client.makeTargetHandle`, which takes:

- The server address
- The provider ID
- The target index (0 for the first target)

We can then use :code:`target.create()` to create a region, which returns
a region ID that can be used for subsequent read/write operations.

Available backends
------------------

Warabi supports multiple storage backends:

- **memory**: In-memory storage (fast, volatile)
- **pmem**: Persistent memory storage (fast, persistent, requires PMDK)
- **abt-io**: Async I/O using ABT-IO (persistent, good for large blobs)
- **dummy**: No-op backend for testing

The backend is specified in the configuration when creating a provider.
Later tutorials will cover each of these backends in detail.

Thread safety and pools
------------------------

Warabi is built on Thallium, which uses Argobots for threading. You can
specify custom Argobots pools for RPC handlers:

.. code-block:: cpp

   tl::pool my_pool = /* ... */;
   warabi::Provider provider(engine, 42, config, my_pool);

This allows fine-grained control over thread/task placement and scheduling.

Error handling
--------------

The C++ API uses exceptions for error handling:

.. code-block:: cpp

   try {
       warabi::TargetHandle target = client.makeTargetHandle(addr, provider_id, 0);
       // Use target...
   } catch(const warabi::Exception& ex) {
       std::cerr << "Warabi error: " << ex.what() << std::endl;
   }

Always wrap Warabi operations in try-catch blocks to handle errors gracefully.

Compile and run
---------------

To compile the server example:

.. code-block:: console

   $ g++ -std=c++14 -o server server.cpp \\
       $(pkg-config --cflags --libs warabi-server thallium)

   $ ./server
   Warabi provider running at na+sm://12345-0

To compile and run the client:

.. code-block:: console

   $ g++ -std=c++14 -o client client.cpp \\
       $(pkg-config --cflags --libs warabi-client thallium)

   $ ./client na+sm://12345-0 42
   Created region with ID: 12345678

Next steps
----------

- :doc:`02_basics`: Learn basic CRUD operations on regions
- :doc:`03_backends_memory`: Learn about the memory backend
- :doc:`10_bedrock`: Learn about Bedrock configuration options
