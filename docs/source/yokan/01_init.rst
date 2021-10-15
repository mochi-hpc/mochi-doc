Getting started with Yokan
==========================

Installing Yokan
----------------

Yokan can be installed using Spack as follows.

.. code-block:: console

   spack install mochi-yokan +bedrock

The :code:`+bedrock` variant will enable :ref:`Bedrock` support, which will be useful
for spinning up a Yokan server without having to write code.
The :code:`spack info mochi-yokan` command can be used to show the list of variants
available. Many variants refer to database backend types. They are disabled by
default. In this tutorial we will use the *map* backend, which is natively available
in Yokan, but feel free to try these tutorials with other backends!

In the following sections, the code can be compiled and linked against the *yokan-server*,
*yokan-client*, and *yokan-admin* libraries, which can be found either by calling
:code:`find_package(yokan)` in CMake, or :code:`pkg-config --libs --cflags yokan-server`
(respectively :code:`yokan-client` and :code:`yokan-admin`) with PkgConfig.

Instantiating a Yokan provider
------------------------------

Yokan adopts the typical Mochi microservice architecture
(used for instance in the :ref:`Margo microservice template<margo-microservice-template>`),
with a *server* library providing the microservice's *provider* implementation,
a *client* library providing access to its capabilities (e.g., putting and getting key/value pairs),
and an *admin* library providing control over the providers (creating and destroying databases).
Hence the first thing we need to do is instantiate a provider.

Since we have enabled Bedrock support, let's take advantage of that and write
a *config.json* file for Bedrock to use (if you are not familiar with Bedrock,
I highly recommand you to read the :ref:`Bedrock` section. Using Bedrock will save
you development time since it allows you to bootstrap a Mochi service using a JSON file
instead of writing code).

.. literalinclude:: ../../../code/yokan/01_init/bedrock.json
   :language: json

We can now give this config file to Bedrock as follows.

.. code-block:: console

   $ bedrock na+sm -c config.json
   [2021-10-14 10:16:17.529] [info] [yokan] YOKAN provider registration done
   [2021-10-14 10:16:17.530] [info] Bedrock daemon now running at na+sm://8551-0


We now have a Yokan provider running, with a *provider id* of 42.

If you need to create a provider in C (either because you don't want to use Bedrock
or because you want your provider to be embedded into an existing application), the
following code shows how to do that.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/yokan/01_init/server.c
       :language: cpp


Using the admin library to create a database
--------------------------------------------

Now that we have our server running, let's write some code
using the *admin* library to create a database.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          admin.c (show/hide)

    .. literalinclude:: ../../../code/yokan/01_init/admin.c
       :language: cpp

After the typical Margo initialization, we lookup the address of
the server from the string address provided in :code:`argv[1]`.
We then create a :code:`yk_admin_t` object, which we initialize
with :code:`yk_admin_init`.

By using :code:`yk_open_database`,
the admin sends an RPC to the provider requesting it to open
a database of type :code:`map`, which is an in-memory key/value
store implemented using C++ :code:`std::map`. Since this database
is in memory, this *open* operation will actually create it.

The :code:`yk_open_database` function's last parameter is a pointer
to the returned :code:`yk_database_id_t` referencing the database.
These identifiers are unique and can be serialized into a 37-byte
null-terminated string using :code:`yk_database_id_to_string`.
We print this identifier before calling :code:`yk_admin_finalize`
to finalize the admin.

This code, once compiled, can be called as follows (changing the
address as needed).


.. code-block:: console

   $ ./admin na+sm://8972-0 42
   Database id is 2e7ca988-9681-43f7-9ef3-ff4c941fbefd (take note of it!)


Interacting with the database via the client interface
------------------------------------------------------

Now let the fun start. We can use the client library to create a client
object, create a database handle, and start interacting with our database.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/yokan/01_init/client.c
       :language: cpp

The client is created using :code:`yk_client_init`. We then convert
the string database id into a :code:`yk_database_id_t` using
:code:`yk_database_id_from_string`, and create a :code:`yk_database_handle_t`
by passing it to :code:`yk_database_handle_create`. This handle
is the object that will let us interact with the database.

As an example of using the client API, we show the use of
:code:`yk_put` and :code:`yk_get` to respectively put and
get a key/value pair from the database. These functions will be
detailed more in the next tutorial.

:code:`yk_database_handle_release` should be called to destroy
the database handle. :code:`yk_client_finalize` is then used
to finalize the client.

This program can be called as follows (changing the address and
the database id as needed).

.. code-block:: console

   $ ./client na+sm://8972-0 42 2e7ca988-9681-43f7-9ef3-ff4c941fbefd
