Basic operations
================

This tutorial covers the fundamental operations for working with Warabi regions:
creating, writing, reading, and destroying regions.

Creating regions
----------------

A region is a container for data with a fixed size. Before you can store data,
you must create a region by specifying its size:

.. code-block:: cpp

   #include <warabi/Client.hpp>

   // Assuming you have a target handle
   warabi::TargetHandle target = /* ... */;

   // Create a region with a fixed size (1 MB in this example)
   warabi::RegionID region_id;
   size_t region_size = 1024 * 1024;  // 1 MB
   target.create(&region_id, region_size);

   std::cout << "Created region: " << region_id << std::endl;

The :code:`create()` method takes a size parameter and returns a region ID,
which is a unique identifier you'll use for all subsequent operations on this region.

**Important**: Regions have a **fixed size** specified at creation time. They do
not grow dynamically. Plan your region sizes accordingly.

**Region IDs**: A :code:`warabi::RegionID` is an opaque identifier. You should
treat it as a handle and not make assumptions about its internal structure.

Writing data to a region
-------------------------

Once you have a region, you can write data to it:

.. code-block:: cpp

   #include <string>
   #include <vector>

   // Data to write
   std::string data = "Hello, Warabi!";

   // Write to the region (volatile write)
   target.write(region_id, 0, data.data(), data.size());

The :code:`write()` method takes:

- Region ID
- Offset (where to start writing)
- Data pointer
- Size of data
- Optional persist flag (default: false)

**Write with persistence**: You can request the data to be persisted immediately:

.. code-block:: cpp

   // Write and persist in one operation
   target.write(region_id, 0, data.data(), data.size(), true);

When ``persist`` is ``false`` (the default), the write is volatile and may only
be in cache. To ensure durability, either use ``persist=true`` or call ``persist()``
separately.

**Bulk writes**: For large data, Warabi uses RDMA bulk transfers automatically:

.. code-block:: cpp

   std::vector<char> large_data(1024 * 1024);  // 1 MB
   // Fill data...

   target.write(region_id, 0, large_data.data(), large_data.size());

Warabi will efficiently transfer the data using Mercury's bulk transfer capabilities.

Reading data from a region
---------------------------

To read data back from a region:

.. code-block:: cpp

   // Buffer to read into
   std::vector<char> buffer(data.size());

   // Read from the region
   target.read(region_id, 0, buffer.data(), buffer.size());

   std::string result(buffer.begin(), buffer.end());
   std::cout << "Read: " << result << std::endl;

The :code:`read()` method takes:

- Region ID
- Offset (where to start reading)
- Buffer pointer
- Size to read

**Partial reads**: You can read portions of a region:

.. code-block:: cpp

   // Read bytes 10-19 (10 bytes starting at offset 10)
   std::vector<char> partial(10);
   target.read(region_id, 10, partial.data(), 10);

Persisting data
---------------

By default, writes are volatile and may only exist in cache. To ensure data
is persisted to durable storage, use the :code:`persist()` function:

.. code-block:: cpp

   // Write data (volatile)
   target.write(region_id, 0, data.data(), data.size());

   // Persist the written data
   target.persist(region_id, 0, data.size());

The :code:`persist()` function takes:

- Region ID
- Offset (where to start persisting)
- Size (how much to persist)

**Partial persistence**: You can persist specific portions of a region:

.. code-block:: cpp

   // Persist only bytes 100-199
   target.persist(region_id, 100, 100);

**Note**: For backends like ``memory``, persist is a no-op. For ``pmem`` and
``abtio`` backends, it ensures data is flushed to persistent storage.

Combined create and write
--------------------------

For convenience, Warabi provides :code:`createAndWrite()` which combines region
creation and writing in a single operation:

.. code-block:: cpp

   // Create region and write data in one operation
   warabi::RegionID region_id;
   std::string data = "Hello, Warabi!";
   target.createAndWrite(&region_id, data.data(), data.size());

You can also request immediate persistence:

.. code-block:: cpp

   // Create, write, and persist
   target.createAndWrite(&region_id, data.data(), data.size(), true);

This is more efficient than calling :code:`create()` and :code:`write()`
separately, especially for small regions.

Destroying regions
------------------

Regions can be deleted as follows.

.. code-block:: cpp

   target.erase(region_id);

**Important**: After destroying a region, its ID becomes invalid. Attempting
to use it will result in an error.

Complete example
----------------

Here's an example demonstrating some of the basic operations:

.. literalinclude:: ../../../code/warabi/02_basics/client.cpp
   :language: cpp

Non-blocking operations
-----------------------

Most of the API presented above accepts an optional pointer to a ``warabi::AsyncRequest``.
If such an object is provided, the method will return immediately and the operation
will be performed in a non-blocking manner. The request object can be used to test
whether the operation has completed (non-blocking) or wait for the operation to complete
(blocking).

Region persistence
------------------

The persistence of regions depends on the backend:

- **memory backend**: Regions are volatile and lost when the provider stops
- **pmem backend**: Regions persist across provider restarts
- **abt-io backend**: Regions are stored on disk and persist

If you need persistent storage, use the pmem or abt-io backends (covered in
later tutorials).

Non-contiguous region accesses
------------------------------

Variants of the ``read``, ``write``, and ``persist`` functions exist that
take a ``std::vector<std::pair<size_t,size_t>>`` list of offset/size pairs
for non-contiguous accesses to the data within a region.

Non-contiguous access to/from a region to/from non-contiguous memory
is also possible by relying on a `thallium::bulk` exposing non-contiguous
user memory.

Region naming
-------------

Region IDs are opaque and generated by Warabi. If you need human-readable
names, you can maintain a mapping yourself:

.. code-block:: cpp

   #include <map>

   std::map<std::string, warabi::RegionID> region_map;

   // Create and name a region
   warabi::RegionID id;
   target.create(&id);
   region_map["my_checkpoint"] = id;

   // Later, retrieve by name
   target.write(region_map["my_checkpoint"], 0, data, size);

Alternatively, you can store the region ID in a metadata service like Yokan.

.. note::

   There is no API to retrieve the size of a region, hence you will also
   need to store it somewhere if needed.

Error handling
--------------

Always handle errors when working with regions:

.. code-block:: cpp

   try {
       target.write(region_id, 0, data, size);
   } catch(const warabi::Exception& ex) {
       std::cerr << "Write failed: " << ex.what() << std::endl;
       // Handle error (retry, log, etc.)
   }

Common errors:

- **Invalid region ID**: The region doesn't exist or was destroyed
- **Out of bounds**: Offset + size exceeds backend limits
- **Backend error**: Storage backend failed (disk full, pmem error, etc.)
- **Network error**: Communication with provider failed

Best practices
--------------

**1. Specify appropriate region sizes**: Since regions have fixed size, plan ahead

.. code-block:: cpp

   // Bad: Creating region too small
   warabi::RegionID id;
   target.create(&id, 100);  // Only 100 bytes
   target.write(id, 0, large_data, 10000);  // ERROR: exceeds region size!

   // Good: Create region large enough
   target.create(&id, large_data_size);
   target.write(id, 0, large_data, large_data_size);

**2. Clean up regions**: Always destroy regions when done to avoid leaks

**3. Persist when needed**: Use ``persist()`` or ``persist=true`` for durability

**4. Handle partial operations**: For very large data, consider splitting into chunks

Performance considerations
--------------------------

**Bulk transfers**: Warabi automatically uses RDMA for large transfers.
For best performance with large data:

.. code-block:: cpp

   // Good: Single large write
   target.write(region_id, 0, large_buffer, 1024*1024);

   // Less efficient: Many small writes
   for(int i = 0; i < 1024; i++) {
       target.write(region_id, i*1024, small_buffer, 1024);
   }

**Alignment**: Some backends (especially pmem) benefit from aligned writes:

.. code-block:: cpp

   // Align to 4KB boundaries for best performance
   const size_t alignment = 4096;
   size_t aligned_offset = (offset / alignment) * alignment;

**Async operations**: For better performance, use async operations (covered in :doc:`09_async`).
