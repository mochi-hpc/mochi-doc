Python Bindings
===============

Warabi provides comprehensive Python bindings for both server and client libraries,
enabling blob storage functionality in Python applications.

Installation
------------

To use Warabi's Python bindings, install Warabi with the ``+python`` variant in Spack:

.. code-block:: bash

   spack install mochi-warabi+python

The Python bindings are available in the ``mochi.warabi`` package.

Quick Start
-----------

Here's a simple example of using Warabi from Python:

.. literalinclude:: ../../../code/warabi/12_python/quickstart.py
   :language: python

This example demonstrates:

1. Creating a Margo engine
2. Starting a Warabi provider with memory backend
3. Creating a client and target handle
4. Creating regions and writing/reading data

Server API
----------

The ``mochi.warabi.server`` module provides the ``Provider`` class for creating
Warabi providers:

Basic Provider Creation
~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../code/warabi/12_python/server_basic.py
   :language: python

Backend Configuration
~~~~~~~~~~~~~~~~~~~~~

Warabi supports multiple storage backends:

.. literalinclude:: ../../../code/warabi/12_python/server_backends.py
   :language: python

Available backends:

- **memory**: Fast in-memory storage (volatile)
- **pmem**: Persistent memory storage
- **abtio**: File-based storage using ABT-IO

Client API
----------

The ``mochi.warabi.client`` module provides the ``Client`` and ``TargetHandle``
classes for interacting with Warabi providers.

Basic Operations
~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../code/warabi/12_python/client_basic.py
   :language: python

Key operations:

- ``create(size)``: Create a new region
- ``write(region, offset, data, persist=False)``: Write data to a region
- ``read(region, offset, size)``: Read data from a region
- ``create_and_write(data, persist=False)``: Create region and write in one operation
- ``erase(region)``: Delete a region
- ``persist(region, offset, size)``: Ensure data is persisted to storage

Working with Regions
--------------------

Regions are logical containers for blob data:

.. literalinclude:: ../../../code/warabi/12_python/regions.py
   :language: python

Regions support:

- Fixed-size allocation
- Offset-based read/write operations
- Persistence control
- Efficient data management

Buffer Protocol Support
-----------------------

Like Yokan, Warabi supports Python's buffer protocol for efficient zero-copy operations:

.. literalinclude:: ../../../code/warabi/12_python/buffer_protocol.py
   :language: python

Using buffer protocol objects (``bytearray``, ``memoryview``, NumPy arrays) avoids
memory copies and improves performance for large data transfers.

Asynchronous Operations
-----------------------

Warabi provides async operations for non-blocking I/O:

.. literalinclude:: ../../../code/warabi/12_python/async_operations.py
   :language: python

Async operations return ``AsyncRequest`` or ``AsyncCreateRequest`` objects that
can be used to:

- Wait for completion with ``wait()``
- Test completion with ``completed()``

Persistence Control
-------------------

Control when data is persisted to storage:

.. literalinclude:: ../../../code/warabi/12_python/persistence.py
   :language: python

Persistence strategies:

- **Immediate**: ``persist=True`` in write operations
- **Explicit**: Call ``persist()`` after writes
- **Batch**: Group multiple writes, then persist once
- **Async**: Use ``persist_async()`` for non-blocking persistence

Working with NumPy Arrays
--------------------------

Warabi integrates seamlessly with NumPy:

.. literalinclude:: ../../../code/warabi/12_python/numpy_example.py
   :language: python

This is useful for:

- Scientific computing workflows
- Machine learning model storage
- Large array datasets
- HPC applications

Advanced Patterns
-----------------

Batch Operations
~~~~~~~~~~~~~~~~

Efficient batch processing:

.. literalinclude:: ../../../code/warabi/12_python/batch_operations.py
   :language: python

Data Migration Pattern
~~~~~~~~~~~~~~~~~~~~~~

Migrate data between storage backends:

.. literalinclude:: ../../../code/warabi/12_python/migration_pattern.py
   :language: python

Performance Tips
----------------

1. **Use buffer protocol objects** instead of strings for large data

2. **Pre-allocate regions** when sizes are known to avoid multiple allocations

3. **Batch writes** when possible to reduce network round-trips

4. **Use async operations** to overlap I/O with computation

5. **Choose appropriate backends**:
   - Memory backend for temporary data
   - ABT-IO backend for persistent storage
   - PMem backend for byte-addressable persistence

6. **Control persistence** explicitly for better performance:
   - Use ``persist=False`` for writes
   - Batch multiple writes
   - Call ``persist()`` once at the end

7. **Preallocate buffers** for read operations to avoid allocations

Integration Examples
--------------------

With Bedrock
~~~~~~~~~~~~

Using Warabi through Bedrock in Python:

.. literalinclude:: ../../../code/warabi/12_python/with_bedrock.py
   :language: python

Building Higher-Level APIs
---------------------------

Create application-specific abstractions:

.. literalinclude:: ../../../code/warabi/12_python/higher_level_api.py
   :language: python

This allows you to:

- Add domain-specific interfaces
- Implement caching strategies
- Add validation and type checking
- Create convenient abstractions
