Getting started with Warabi
===========================

Installing Warabi
-----------------

Warabi can be installed using Spack as follows.

.. code-block:: console

   spack install mochi-warabi +bedrock +python

The :code:`+bedrock` variant will enable :ref:`Bedrock` support, which will be useful
for spinning up a Warabi provider without having to write code. ``+python`` is used
to enable Python support.
The :code:`spack info mochi-warabi` command can be used to show the list of variants
available.

In the following sections, the code can be compiled and linked against the *warabi-server*
and *warabi-client* libraries, which can be found either by calling
:code:`find_package(warabi)` in CMake and linking against the ``warabi::client`` and
``warabi::server`` targets, or :code:`pkg-config --libs --cflags warabi-client`
(respectively :code:`warabi-server`) with PkgConfig.

Key concepts
------------

**Targets**: A target is a storage endpoint that can hold regions. Each provider
manages one target.

**Regions**: A region is a logical container for data. You create regions,
write data to them, and read data from them. Each region has a unique ID
assigned by Warabi upon creation.

**Transfer Managers**: Control how data is transferred between client and server,
with options for pipelining and concurrency.

**Backends**: The underlying storage implementation (memory, pmem, abt-io, etc.).

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

.. literalinclude:: ../../../code/warabi/01_intro/bedrock-config.json
   :language: json

We can now give this config file to Bedrock as follows.

.. code-block:: console

   $ bedrock na+sm -c bedrock-config.json
   [info] Bedrock daemon now running at na+sm://12345-0

We now have a Warabi provider running, with a *provider id* of 42, managing
a target using the "memory" backend.

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

We can then use :code:`target.create()` to create a region, which returns
a region ID that can be used for subsequent read/write operations.

RPC Pool
--------

Warabi providers can take an optional pool in which to execute their RPC,
as follows.

.. code-block:: cpp

   tl::pool my_pool = /* ... */;
   warabi::Provider provider(engine, 42, config, my_pool);

Using Bedrock, this is done by providing the name of a pool as a dependency,
as follows.

.. code-block:: json

   {
       "type": "warabi",
       "name": "my_warabi_provider",
       ...
       "dependencies": {
           "pool": "my_pool"
       }
   }


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
