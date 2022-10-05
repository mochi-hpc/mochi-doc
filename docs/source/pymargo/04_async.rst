Non-blocking calls
==================

PyMargo RPC calls, as well as calls to :code:`respond` and
:code:`transfer`, can be made non-blocking by passing :code:`blocking=False`
as parameter. This will make these functions return a :code:`Request` object,
which has a :code:`test()` and a :code:`wait()` function. The former
returns whether the request has completed, without blocking. The latter
actually blocks until the request completed.

The server and client codes bellow examplify the use of non-blocking calls.

.. literalinclude:: ../../../code/pymargo/04_non_blocking/server.py
   :language: python

.. literalinclude:: ../../../code/pymargo/04_non_blocking/client.py
   :language: python

.. important::
   :code:`wait` needs to be called even if :code:`test` returned :code:`True`.
   For starter, calling :code:`wait` on an RPC request is what will actually
   return the RPC's response, but this is also what will make the :code:`Request`
   object free its internal resources.
