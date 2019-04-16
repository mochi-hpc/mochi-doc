Initializing Margo
==================

In this tutorial, we will see how to initialize Margo.

Initializing a server
---------------------

The following code initializes Margo for use as a server,
then prints the address at which the server can be contacted.

.. container:: toggle

    .. container:: header
    
       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/margo/01_init/server.c
       :language: cpp

:code:`margo_init` creates a :code:`margo_instance_id` object. It takes four arguments.
The first one is *protocol* (here TCP). It is also possible to provide the address and port number to use.
The second argument specifies that Margo is initialized as a server.
The third argument indicates whether an Argobots execution stream (ES) should be created to run the
Mercury progress loop. If this argument is set to 0, the progress loop is going to run in the
context of the main ES (this should be the standard scenario, unless you have a good reason for not
using the main ES, such as the main ES using MPI primitives that could block the progress loop). A value
of 1 will make Margo create an ES to run the Mercury progress loop.
The fourth argument is the number of ES to create and use for executing RPC handlers. A value of 0 will
make Margo execute RPCs in the ES that called :code:`margo_init`. A value of -1 will make Margo execute the
RPCs in the ES running the progress loop. A positive value will make Margo create new ESs to run the RPCs.

:code:`margo_addr_self` is then used to get the address of the server, which is then converted into a string
using :code:`margo_addr_to_string`. The address returned by :code:`margo_addr_self` should be freed using
:code:`margo_addr_free`.

:code:`margo_wait_for_finalize` blocks the server in the Mercury progress loop until another ES calls
:code:`margo_finalize`. In this example, nothing calls :code:`margo_finalize`, so you will need to kill the server
manually if you run it.


Initializing a client
---------------------

The following code initializes Margo for use as a client.

.. container:: toggle

    .. container:: header
    
       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/margo/01_init/client.c
       :language: cpp

We call :code:`margo_init` with :code:`MARGO_CLIENT_MODE` to indicate that this is a client.
Just like servers, clients have to run a Mercury progress loop. This progress loop can be
executed in the context of the main ES (by providing 0 as a third argument) or in a separate
ES (by providing 1). In general, a value of 0 is sufficient. Putting the Mercury progress
loop of a client in a separate ES is useful if the client uses non-blocking RPCs,
or if the client is multithreaded.

:code:`margo_finalize` is used to finalize the :code:`margo_instance_id` object.
