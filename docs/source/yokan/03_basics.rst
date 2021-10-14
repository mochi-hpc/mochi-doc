Basics of accessing a database
==============================

Having started a provider with a database, we are now
ready for clients to connect and access it.
The following program showcases the use of the
database handle in the client library to access
single key/value pairs.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/yokan/03_basic/client.c
       :language: cpp

The client library provides the following functions for accessing
single key/value pairs:

- :code:`yk_put`: puts a key/value pair into the database.
- :code:`yk_count`: counts the number of key/value pairs currently
  in the database.
- :code:`yk_exists`: checks whether the given key is in the database.
- :code:`yk_length`: gets the length of the value associated with a given key.
- :code:`yk_get`: gets the value associated with the given key.
- :code:`yk_erase`: erases the key/value pair associated with the given key.

These functions are extensively documented in the *yokan/database.h* header.

If your application accesses a large number of key/value pairs and
can batch them together, we recommend using the functions presented
in the next tutorial to improve performance.
