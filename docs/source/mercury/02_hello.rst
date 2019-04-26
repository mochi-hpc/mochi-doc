Simple Hello World RPC
======================

In this tutorial, we will register an RPC that simply
prints "Hello World" on the server's standard output.

Server code
-----------

The following code shows how to register the RPC on the server.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/mercury/02_hello/server.c
       :language: cpp

To register a function as an RPC, it must take an :code:`hg_handle_t`
as argument and return a value of type :code:`hg_return_t`
(typically :code:`HG_SUCCESS` if the handler executed correctly).

This function (:code:`hello_workd` in our case) is registered as
an RPC using :code:`HG_Register_name`. Which returns an identifier
for the RPC. We also call :code:`HG_Registered_disable_response`
to indicate that this RPC is not going to send any response back
to the client.

Inside the definition of :code:`hello_world`, we simply print "Hello World"
on the standard output, then call :code:`HG_Destroy` to destroy the RPC handle
passed to the function.

Client code
-----------

The following is the corresponding client code.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/mercury/02_hello/client.c
       :language: cpp

Just like in the server, we use :code:`HG_Register_name` to register the RPC,
this time passing NULL instead of a function pointer as last argument.
We also call :code:`HG_Registered_disable_response` to indicate that the server
will not send a response back.

:code:`HG_Addr_lookup` is used to lookup the address of the server. This function
takes a callback as its second argument. This callback must be a function that
takes a :code:`const struct hg_cb_info*` and return a value of type :code:`hg_return_t`.
It will be called when the address lookup completes.

Next, we enter a progress loop similar to that of the server. This is because
we are waiting for :code:`HG_Addr_lookup` to complete. The provided callback
will be executed from inside :code:`HG_Trigger`.

Inside the :code:`lookup_callback` function, we can the get the address of the server
using :code:`callback_info->info.lookup.addr`. This address can be used to create
an instance of RPC using :code:`HG_Create`, and forward it using :code:`HG_Forward`.

Since we don't expect any response, we can immediately call :code:`HG_Destroy` to
destroy the RPC handle we just forwarded. We set :code:`completed` to 1 to exit
the progress loop in :code:`main`.
