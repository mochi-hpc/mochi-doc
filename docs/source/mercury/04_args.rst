RPC arguments and return values
===============================

In the previous tutorials we haven't passed nor returned
any data to/from the RPC handler. In this tutorial we will
see how to send data as arguments to the RPC, and return
data from the RPC.

We will take the example of an RPC that computes the sum
of two numbers sent by the client.

Input/output structure
----------------------

First, we need to declare the types of the RPC arguments
and return values. This is done using the Mercury macros
found in the mercury_macros.h header, as follows.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          types.h (show/hide)

    .. literalinclude:: ../../../code/mercury/04_args/types.h
       :language: cpp

Client code
-----------

The following code looks up the address of the server, then
sends an RPC to the server.


.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/mercury/04_args/client.c
       :language: cpp

The main difference compared with the previous tutorial
is that we pass a pointer to a :code:`sum_in_t` structure
to :code:`HG_Forward`, as well as a completion callback
:code:`sum_completed`. This completion callback will be called
when the server has responded. In this callback, :code:`HG_Get_output`
is used to retrieve the output data sent by the server.
We need to call :code:`HG_Free_output` to free the output after using it.
Note also that :code:`HG_Destroy` is now used in the completion callback
rather than after :code:`HG_Forward`.

Server code
-----------

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/mercury/04_args/server.c
       :language: cpp

On the server side, we use :code:`HG_Get_input` to deserialize the input
data into a :code:`sum_in_t` structure. We use :code:`HG_Free_input`
when we are done with the input data. :code:`HG_Respond` now takes
a pointer to a :code:`sum_out_t` object to return to the client.
