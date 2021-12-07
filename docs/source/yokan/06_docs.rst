Document storage
================

Yokan provides a second set of client functions to use it as a document store.
Contrary to a key/value store, a document store associates values (or "documents")
with a unique, 64-bit identifier chosen by the database. Documents are organized
into collections within a database.


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


Accessing multiple documents
----------------------------

Just like the key/value storage interface, Yokan's document storage
interface provides functions to access multiple documents at once.

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
