RDMA transfers
==============

Mercury can use RDMA to transfer large amounts of data.
In this tutorial we will demonstrate how to use this feature
by transfering the content of a file from a client to a server.

Input/output structures
-----------------------

Like in our earlier examples, we need to define the structures
used for RPC inputs and outputs. These are as follows.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          types.h (show/hide)

    .. literalinclude:: ../../../code/mercury/05_bulk/types.h
       :language: cpp

The client will send the name of the file (:code:`hg_string_t`),
its size (:code:`hg_size_t`), and a bulk handle representing the
region of memory exposed by the client and containing the content
of the file.

The server will simply respond with an integer indicating whether
the operation was succesful.

Client code
-----------

The client code is as follows.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/mercury/05_bulk/client.c
       :language: cpp

We define a :code:`save_operation` structure to keep information
about the on-going operation. This structure will be passed by pointer
as user-provided argument to callbacks.

In the lookup callback, we open the file and read its content into
a buffer. We then use :code:`HG_Bulk_create` to expose the buffer
for RDMA operations. This gives us an :code:`hg_bulk_t` object that
can be sent over RPC to the server.

Once the RPC has completed and a response is received, the :code:`hg_bulkt_t`
object is freed using :code:`HG_Bulk_free`.

Server code
-----------

The following code corresponds to the server.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/mercury/05_bulk/server.c
       :language: cpp

On the server, the :code:`rpc_state` structure will be used to keep
track of information about an on-going operation. In particular, it
contains the :code:`hg_handle_t` object of the on-going RPC, and
the :code:`hg_bulk_t` object of the local buffer exposed to receive
the data.

Upon receiving an RPC, we enter the :code:`save` callback. This
function allocates a local buffer to receive the data and exposes it
using :code:`HG_Bulk_create`.

We issue the RDMA operation using :code:`HG_Bulk_transfer`, specifying
the :code:`HG_BULK_PULL` type of operation, and :code:`save_bulk_completed`
as a callback to call once the the RDMA operation has completed.
It is important to note that this function returns immediately and the RDMA
operation has not be completed at this point. The :code:`save` callback
will return and the Mercury progress loop will continue running, eventually
calling :code:`save_bulk_completed` when the RDMA operation has finished.

Note that we don't respond to the client in the :code:`save` callback,
we do in the :code:`save_bulk_completed` callback, hence the :code:`save`
callback does not destroy the RPC's :code:`hg_handle_t` object. This
object is kept and freed in :code:`save_bulk_completed`.

Ahhh, callbacks... (now you understand how much easier Margo and Thallium are).
