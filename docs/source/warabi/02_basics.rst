Basic operations
================

This tutorial covers the fundamental operations for working with Warabi regions:
creating, writing, reading, and destroying regions.

Creating regions
----------------

A region is a container for data. Before you can store data, you must create a region:

.. code-block:: cpp

   #include <warabi/Client.hpp>

   // Assuming you have a target handle
   warabi::TargetHandle target = /* ... */;

   // Create a region
   warabi::RegionID region_id;
   target.create(&region_id);

   std::cout << "Created region: " << region_id << std::endl;

The :code:`create()` method returns a region ID, which is a unique identifier
you'll use for all subsequent operations on this region.

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

   // Write to the region
   target.write(region_id, 0, data.data(), data.size());

The :code:`write()` method takes:
- Region ID
- Offset (where to start writing)
- Data pointer
- Size of data

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

Querying region size
--------------------

To find out how much data is in a region:

.. code-block:: cpp

   size_t size = target.size(region_id);
   std::cout << "Region contains " << size << " bytes\n";

This is useful when you don't know the size beforehand and want to read
the entire region:

.. code-block:: cpp

   size_t region_size = target.size(region_id);
   std::vector<char> all_data(region_size);
   target.read(region_id, 0, all_data.data(), region_size);

Destroying regions
------------------

When you're done with a region, you should destroy it to free resources:

.. code-block:: cpp

   target.destroy(region_id);

**Important**: After destroying a region, its ID becomes invalid. Attempting
to use it will result in an error.

Complete example
----------------

Here's a complete example demonstrating all basic operations:

.. code-block:: cpp

   #include <thallium.hpp>
   #include <warabi/Client.hpp>
   #include <iostream>
   #include <vector>
   #include <string>

   namespace tl = thallium;

   int main(int argc, char** argv) {
       if(argc != 3) {
           std::cerr << "Usage: " << argv[0] << " <server> <provider_id>\n";
           return -1;
       }

       try {
           // Initialize client
           tl::engine engine("na+sm", THALLIUM_CLIENT_MODE);
           warabi::Client client(engine);

           // Get target handle
           warabi::TargetHandle target = client.makeTargetHandle(
               argv[1], std::atoi(argv[2]), 0
           );

           // Create a region
           warabi::RegionID region_id;
           target.create(&region_id);
           std::cout << "Created region: " << region_id << std::endl;

           // Write data
           std::string message = "Hello, Warabi! This is a test message.";
           target.write(region_id, 0, message.data(), message.size());
           std::cout << "Wrote " << message.size() << " bytes\n";

           // Query size
           size_t size = target.size(region_id);
           std::cout << "Region size: " << size << " bytes\n";

           // Read data back
           std::vector<char> buffer(size);
           target.read(region_id, 0, buffer.data(), size);

           std::string result(buffer.begin(), buffer.end());
           std::cout << "Read: " << result << std::endl;

           // Verify
           if(result == message) {
               std::cout << "SUCCESS: Data matches!\n";
           } else {
               std::cout << "ERROR: Data mismatch!\n";
           }

           // Clean up
           target.destroy(region_id);
           std::cout << "Region destroyed\n";

       } catch(const warabi::Exception& ex) {
           std::cerr << "Error: " << ex.what() << std::endl;
           return -1;
       }

       return 0;
   }

Region persistence
------------------

The persistence of regions depends on the backend:

- **memory backend**: Regions are volatile and lost when the provider stops
- **pmem backend**: Regions persist across provider restarts
- **abt-io backend**: Regions are stored on disk and persist

If you need persistent storage, use the pmem or abt-io backends (covered in
later tutorials).

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

**1. Check region IDs**: Ensure region was created successfully before using it

.. code-block:: cpp

   warabi::RegionID id;
   target.create(&id);
   if(id == warabi::RegionID{}) {  // Check for invalid ID
       std::cerr << "Failed to create region\n";
       return -1;
   }

**2. Clean up regions**: Always destroy regions when done to avoid leaks

**3. Use appropriate buffer sizes**: Allocate buffers based on actual data size

**4. Handle partial operations**: For very large data, consider splitting into chunks

**5. Verify critical data**: For important writes, read back and verify

Performance considerations
--------------------------

**Bulk transfers**: Warabi automatically uses RDMA for large transfers (typically > 4KB).
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

Next steps
----------

- :doc:`03_backends_memory`: Learn about the memory backend
- :doc:`04_backends_pmem`: Learn about persistent memory storage
- :doc:`05_backends_abtio`: Learn about ABT-IO async I/O backend
- :doc:`09_async`: Learn about async operations for better performance
