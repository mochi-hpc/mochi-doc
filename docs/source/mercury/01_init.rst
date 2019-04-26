Initializing Mercury
====================

In this tutorial you will learn how to initialize Mercury as
a client and as a server.

Initializing as a client
------------------------

The following code exemplifies how to initialize Mercury as a client.
We first need to call :code:`HG_Init` to create an :code:`hg_class`
instance, then call :code:`HG_Context_create` to create a context.

This code then immediately calls :code:`HG_Context_destroy` and
:code:`HG_Finalize` to destroy the context and finalize Mercury,
respectively, before terminating.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/mercury/01_init/client.c
       :language: cpp


Initializing as a server
------------------------

The following code exemplifies how to initialize Mercury as a server.
Just like for a client, we call :code:`HG_Init` and :code:`HG_Context_create`,
but this time we pass :code:`HG_TRUE` to :code:`HG_Init` to indicate that
this process is going to listen for incoming requests.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/mercury/01_init/server.c
       :language: cpp

This code also exemplifies a typical Mercury progress loop.
This progress loop alternates :code:`HG_Progress`, which makes progress
on network events (sending and receiving data), and :code:`HG_Trigger`,
which calls registered callbacks based on the events that happened in
:code:`HG_Progress`.

.. note::
   Since this server does not expose any RPC yet, it will just keep
   running until you kill it.
