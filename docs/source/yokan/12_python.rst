Python Bindings
===============

Yokan provides comprehensive Python bindings for both server and client libraries,
allowing you to build and interact with Yokan databases from Python applications.

Installation
------------

To use Yokan's Python bindings, install Yokan with the ``+python`` variant in Spack:

.. code-block:: bash

   spack install mochi-yokan+python

The Python bindings are available in the ``mochi.yokan`` package.

Quick Start
-----------

Here's a simple example of using Yokan from Python:

.. literalinclude:: ../../../code/yokan/12_python/quickstart.py
   :language: python

This example demonstrates:

1. Creating a Margo engine
2. Starting a Yokan provider
3. Creating a client and database handle
4. Performing basic put/get operations

Server API
----------

The ``mochi.yokan.server`` module provides the ``Provider`` class for creating
Yokan providers:

.. literalinclude:: ../../../code/yokan/12_python/server_example.py
   :language: python

Provider configuration uses the same JSON format as the C++ API, supporting
all backend types and configuration options.

Client API
----------

The ``mochi.yokan.client`` module provides the ``Client`` and database handle
for interacting with Yokan providers:

Basic Operations
~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../code/yokan/12_python/client_basic.py
   :language: python

The client API supports:

- ``put(key, value, mode=0)``: Store a key/value pair
- ``get(key, value, mode=0)``: Retrieve a value by key
- ``exists(key, mode=0)``: Check if a key exists
- ``length(key, mode=0)``: Get the size of a value
- ``erase(key, mode=0)``: Delete a key/value pair
- ``count(mode=0)``: Count total key/value pairs

.. note::
   The ``get`` method does not return the value, instead an appropriately-sized
   buffer needs to be passed in which the value will be stored. This is because
   Yokan optimizes operations by doing RDMA directly to the target memory.

Batch Operations
~~~~~~~~~~~~~~~~

For efficiency, use batch operations when working with multiple keys:

.. literalinclude:: ../../../code/yokan/12_python/batch_operations.py
   :language: python

Batch operations include:

- ``put_multi(pairs, mode=0)``
- ``get_multi(pairs, mode=0)``
- ``exists_multi(keys, mode=0)``
- ``length_multi(keys, mode=0)``
- ``erase_multi(keys, mode=0)``

.. note::
   The ``put_multi`` and ``get_multi`` methods take a ``list`` of ``pairs``,
   with the first element of the pair being the key, and the second being the
   value, or a destination buffer for the value.

List Operations
~~~~~~~~~~~~~~~

Yokan provides powerful list operations for iterating through key/value pairs:

.. literalinclude:: ../../../code/yokan/12_python/list_operations.py
   :language: python

The Buffer Protocol
-------------------

All functions in the Python bindings accept either strings or any object
that implements the `buffer protocol <https://docs.python.org/3/c-api/buffer.html>`_.

Strings may only be used as input (e.g. the key in ``put`` and ``get``, and the
value in ``put``, but not the value in ``get``).

Working with Modes
------------------

Yokan's mode system is available in Python through the ``mochi.yokan.mode`` module:

.. literalinclude:: ../../../code/yokan/12_python/modes_example.py
   :language: python

Available modes have the same names as in C/C++.
Modes can be combined using bitwise OR: ``mode.YOKAN_MODE_WAIT | Mode.YOKAN_MODE_CONSUME``

Document Store Operations
--------------------------

Yokan also supports document storage with the collection API:

.. literalinclude:: ../../../code/yokan/12_python/documents.py
   :language: python

Collections provide:

- JSON document storage
- Query capabilities
- Document IDs
- Batch operations

Error Handling
--------------

The Python bindings raise ``mochi.yokan.client.Exception`` for client errors
and ``mochi.yokan.server.Exception`` for server errors:

.. literalinclude:: ../../../code/yokan/12_python/error_handling.py
   :language: python

Always wrap Yokan operations in try/except blocks to handle:

- Missing keys
- Network errors
- Unsupported modes
- Invalid configurations

Performance Tips
----------------

1. **Use batch operations** when working with multiple keys to reduce network round-trips

2. **Use buffer protocol objects** instead of strings for large binary data

3. **Preallocate buffers** when retrieving multiple values to avoid allocations

4. **Choose appropriate batch sizes** for list operations based on your data size

5. **Use YOKAN_MODE_NO_RDMA** for small key/value pairs to avoid RDMA overhead

6. **Consider backend selection** based on your workload (in-memory vs persistent)

7. **Use CONSUME mode** when you need get-and-delete atomicity

Building Higher-Level APIs
---------------------------

The Python bindings are designed to be low-level and performance-oriented.
We encourage users to build higher-level APIs for their use cases:

.. literalinclude:: ../../../code/yokan/12_python/higher_level_api.py
   :language: python

This allows you to:

- Add application-specific abstractions
- Implement custom caching
- Add type checking and validation
- Create domain-specific interfaces
