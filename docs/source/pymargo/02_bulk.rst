Bulk transfers
==============

In the following, we will see how to use bulk (RDMA) transfers
in PyMargo. Bellow are the example server and client codes we
will use.

Server
------

.. literalinclude:: ../../../code/pymargo/02_bulk/server.py
   :language: python

Client
------

.. literalinclude:: ../../../code/pymargo/02_bulk/client.py
   :language: python

The above server registers a :code:`do_bulk_transfer` method
of a :code:`Receiver` instance (we use such a class just for
the convenience of keeping the engine as an instance variable.
This also shows how a code can be structured around a class
that provides RPC functionalities). This :code:`do_bulk_transfer`
method takes a :code:`Bulk` object among other arguments.
This object represents a handle to a region of memory exposed
by the client. The :code:`do_bulk_transfer` function first
creates a :code:`bytes` buffer of the same size to use as target
for a PULL operation. It creates a local :code:`Bulk` object
using the engine's :code:`create_bulk` function, then use
:code:`transfer` to pull the data from the client's memory to
the server's buffer. Note that this transfer function can
transfer to and from any part of the origin and local buffer
by specifying different offsets.

On the client side, we also create a :code:`Bulk` object from
a local buffer, and pass it as argument to the RPC.

.. note::
   In the code above we have used :code:`bytes` objects as buffers.
   The ::code:`create_bulk` function can however work with any
   object that satisfies the
   `buffer protocol <https://docs.python.org/3/c-api/buffer.html>`_,
   such as :code:`bytearray`, :code:`array.array`, numpy arrays, etc.,
   provided that the buffer's memory is contiguous.
