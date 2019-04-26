Passing around a context
========================

The previous tutorial used global static variables to make
things like the :code:`hg_context` and :code:`hg_class`
accessible from within callbacks. Since any good developer would ban
such a practice, we will revisit the previous tutorial with local
variables instead.

Client
------

On the client side, we encapsulate a context in the :code:`client_data_t`
structure. By passing a pointer to this structure as third argument
of :code:`HG_Addr_lookup`, we can recover it as :code:`callback_info->arg`
in the callback. This lets us carry the :code:`hg_class`, :code:`hg_context`,
and :code:`hello_rpc_id` from :code:`main` to the :code:`lookup_callback`
function.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/mercury/03_local/client.c
       :language: cpp

Server
------

On the server side, we encapsulate our information in a :code:`server_data_t`
structure. We use :code:`HG_Register_data` to attach a pointer to the structure
to the RPC handler (the fourth argument, :code:`NULL`, corresponds to a function
to be called to free the pointer when the RPC handler is deregistered. Since our
structure is on the stack, we do not need to provide any such function).

Within the :code:`hello_world` handler, we recover the pointer to our
:code:`server_data_t` structure by using :code:`HG_Get_info` and
:code:`HG_Registered_data`.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/mercury/03_local/server.c
       :language: cpp
