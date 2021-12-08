Document storage
================

Yokan provides a second set of client functions to use a database as a document store.
Contrary to a key/value store, a document store associates values (or "documents")
with a unique, 64-bit identifier chosen by the database, and incrementally increasing
from 0. Documents are organized into named collections within a database.

All the Yokan backends that are *sorted* offer a document-storage interface on top
of them.

.. important::
   It is recommended not to use both the document storage and the key/value storage
   interface on the same database. Since the document storage functionalities rely
   on storing key/value pairs, modifying without a good understanding of how Yokan
   maps documents to key/value pairs could corrupt the document storage.


Manipulating collections
------------------------

The following code shows how to create a collection, check whether a collection
exists, get the collection size, the last identifier, and drop a collection.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/yokan/05_coll/client.c
       :language: cpp

These functions are extensively documented in the *yokan/collection.h* header.
Contrary to database creation and management, collection creation and management
functions are available to the client library.


Accessing single documents
--------------------------

The following code shows how to store, load, update, get the length of,
and erase a document.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/yokan/06_doc/client.c
       :language: cpp

The :code:`yk_id_t` type is typedef-ed as a :code:`uint64_t`.

.. note::
   For small documents (less than a few KB), it is recommended to
   use the :code:`YOKAN_MODE_NO_RDMA` mode, which will pack the
   document into the RPC message instead of relying on RDMA.

.. note::
   Just like in the key/value storage interface, if you have
   multiple documents to access, we recommend using the functions
   bellow, which are designed to work on batches of documents.


Accessing multiple documents
----------------------------

The following shows how to access a batch of documents when the
buffers used for each document are not contiguous in memory.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client-multi.c (show/hide)

    .. literalinclude:: ../../../code/yokan/07_doc_multi/client-multi.c
       :language: cpp

A second set of functions is provided to work with documents packed
contiguously in memory. We strongly advise to use these "packed"
functions for better performance.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client-packed.c (show/hide)

    .. literalinclude:: ../../../code/yokan/07_doc_multi/client-packed.c
       :language: cpp

.. important::
   The :code:`YOKAN_MODE_NO_RDMA` will not work with the :code:`_multi`
   version of these functions, but will work with the :code:`_packed`
   version. This mode may be more efficient, in particular when documents
   are small or when the server is facing a high degree of concurrency.
