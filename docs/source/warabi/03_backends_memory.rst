Backend: memory
===============

The memory backend stores data in RAM, providing the fastest access but with
no persistence across provider restarts.

When to use
-----------

Use the memory backend when:

- You need maximum performance
- Data is temporary or can be regenerated
- You're implementing caching
- You're testing or prototyping
- Persistence is not required

Characteristics
---------------

**Fast**: All operations are in-memory with no I/O overhead

**Volatile**: Data is lost when the provider stops

**No size limits**: Limited only by available RAM

**Simple**: No configuration needed

Configuration
-------------

In Bedrock configuration:

.. code-block:: json

   {
       "providers": [{
           "type": "warabi",
           "provider_id": 42,
           "config": {
               "targets": [
                   {
                       "type": "memory",
                       "config": {}
                   }
               ]
           }
       }]
   }

The memory backend has no configuration options - just use an empty :code:`config` object.

In C++ code:

.. code-block:: cpp

   #include <warabi/Provider.hpp>

   auto config = R"(
   {
       "targets": [{
           "type": "memory",
           "config": {}
       }]
   }
   )";

   warabi::Provider provider(engine, 42, config);

Multiple targets
----------------

You can create multiple memory targets in one provider:

.. code-block:: json

   {
       "config": {
           "targets": [
               {"type": "memory", "config": {}},
               {"type": "memory", "config": {}},
               {"type": "memory", "config": {}}
           ]
       }
   }

Each target is independent and has its own set of regions. Access them using
different target indices:

.. code-block:: cpp

   warabi::TargetHandle target0 = client.makeTargetHandle(addr, provider_id, 0);
   warabi::TargetHandle target1 = client.makeTargetHandle(addr, provider_id, 1);
   warabi::TargetHandle target2 = client.makeTargetHandle(addr, provider_id, 2);

Memory management
-----------------

The memory backend allocates memory dynamically as regions are created and
written to:

.. code-block:: cpp

   // Create region (no allocation yet)
   warabi::RegionID id;
   target.create(&id);

   // Write data (memory allocated now)
   std::vector<char> data(1024 * 1024);  // 1 MB
   target.write(id, 0, data.data(), data.size());

Memory is freed when:
- A region is destroyed: :code:`target.destroy(id)`
- The provider is shut down

**Memory growth**: Regions can grow as you write to them:

.. code-block:: cpp

   target.write(id, 0, data1, 1024);        // Region is 1 KB
   target.write(id, 1024, data2, 1024);     // Region grows to 2 KB
   target.write(id, 100000, data3, 1024);   // Region grows to ~100 KB

The backend allocates memory to accommodate the highest offset written.

Performance characteristics
---------------------------

The memory backend offers excellent performance:

**Write throughput**: Limited only by memory bandwidth and RPC overhead

**Read throughput**: Same as write throughput

**Latency**: Minimal - microseconds for small operations

**Concurrency**: Multiple concurrent operations are safe

Benchmark example:

.. code-block:: cpp

   #include <chrono>

   auto start = std::chrono::high_resolution_clock::now();

   const size_t data_size = 1024 * 1024 * 100;  // 100 MB
   std::vector<char> data(data_size);

   target.write(region_id, 0, data.data(), data.size());

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

   double throughput = (data_size / (1024.0 * 1024.0)) / (duration.count() / 1000.0);
   std::cout << "Throughput: " << throughput << " MB/s\n";

Use cases
---------

**1. Caching**: Store frequently accessed data in memory

.. code-block:: cpp

   // Cache computation results
   std::vector<char> result = expensive_computation();
   target.write(cache_region, 0, result.data(), result.size());

   // Retrieve from cache
   std::vector<char> cached(result.size());
   target.read(cache_region, 0, cached.data(), cached.size());

**2. Temporary storage**: Hold intermediate results in workflows

.. code-block:: cpp

   // Stage 1: Write intermediate results
   target.write(temp_region, 0, intermediate.data(), intermediate.size());

   // Stage 2: Read and process
   std::vector<char> data(size);
   target.read(temp_region, 0, data.data(), size);
   auto final_result = process(data);

   // Clean up
   target.destroy(temp_region);

**3. High-performance scratch space**: Fast temporary storage for HPC applications

.. code-block:: cpp

   // Each MPI rank creates its own scratch region
   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   warabi::RegionID scratch;
   target.create(&scratch);

   // Use for temporary computations
   // ... work ...

   target.destroy(scratch);

**4. Testing**: Verify application logic without I/O overhead

Memory limits
-------------

The memory backend has no built-in limits, but practical limits include:

- **Available RAM**: The system's total memory
- **Provider limits**: Process memory limits (ulimit, cgroups)
- **Address space**: 64-bit systems have vast address space

Monitor memory usage:

.. code-block:: cpp

   // On Linux, check /proc/self/status for memory usage
   std::ifstream status("/proc/self/status");
   std::string line;
   while(std::getline(status, line)) {
       if(line.substr(0, 6) == "VmRSS:") {
           std::cout << line << std::endl;  // Resident memory
       }
   }

Comparison with other backends
-------------------------------

**vs. pmem**:
- Memory: Faster, volatile
- Pmem: Slightly slower, persistent

**vs. abt-io**:
- Memory: Much faster, volatile, unlimited concurrency
- ABT-IO: Slower, persistent, better for very large data

**vs. dummy**:
- Memory: Actual storage
- Dummy: No storage (testing only)

Choose memory when speed is critical and persistence is not needed.

Best practices
--------------

**1. Clean up regions**: Free memory by destroying unused regions

.. code-block:: cpp

   target.destroy(region_id);  // Frees memory immediately

**2. Monitor memory usage**: Track allocation to avoid OOM

**3. Use for appropriate data**: Temporary or cacheable data only

**4. Consider size**: For very large datasets (>GB), consider pmem or abt-io

**5. Combine with persistent storage**: Use memory for hot data, persistent
backends for cold data

Example: Two-tier storage
--------------------------

Combine memory (hot tier) and abt-io (cold tier):

.. code-block:: cpp

   // Hot tier (memory)
   warabi::TargetHandle memory_target = client.makeTargetHandle(addr, pid, 0);

   // Cold tier (abt-io)
   warabi::TargetHandle disk_target = client.makeTargetHandle(addr, pid, 1);

   // Write to both
   target.write(memory_region, 0, data, size);  // Fast cache
   target.write(disk_region, 0, data, size);    // Persistent backup

   // Read from memory (fast path)
   try {
       target.read(memory_region, 0, buffer, size);
   } catch(...) {
       // Fallback to disk if memory region gone
       target.read(disk_region, 0, buffer, size);
   }

Next steps
----------

- :doc:`04_backends_pmem`: Learn about persistent memory storage
- :doc:`05_backends_abtio`: Learn about ABT-IO async I/O backend
- :doc:`06_transfer_managers`: Optimize data transfer strategies
