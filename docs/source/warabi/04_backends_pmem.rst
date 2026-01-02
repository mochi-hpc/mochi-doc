Backend: pmem
=============

The pmem (persistent memory) backend stores data in persistent memory devices,
providing near-memory performance with persistence across provider restarts.

When to use
-----------

Use the pmem backend when:

- You have persistent memory hardware (Intel Optane, etc.)
- You need both high performance and persistence
- Data must survive provider restarts
- You want byte-addressable persistent storage
- Low latency is critical

Prerequisites
-------------

The pmem backend requires:

**Hardware**: Persistent memory devices (Intel Optane DCPMM, NVDIMM, etc.)

**Software**: PMDK (Persistent Memory Development Kit)

.. code-block:: console

   # Install PMDK
   spack install pmdk

   # Install Warabi with pmem support
   spack install mochi-warabi +bedrock +pmem

**Filesystem**: Persistent memory must be mounted with DAX (Direct Access):

.. code-block:: console

   # Mount pmem device
   sudo mount -o dax /dev/pmem0 /mnt/pmem

Characteristics
---------------

**Persistent**: Data survives provider restarts and system reboots

**Fast**: Near-DRAM performance (100s of nanoseconds latency)

**Byte-addressable**: No block-level I/O overhead

**Limited capacity**: Pmem devices are typically smaller than SSDs

**Power-fail safe**: Writes are durable after return

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
                       "type": "pmem",
                       "config": {
                           "path": "/mnt/pmem/warabi",
                           "size": 10737418240
                       }
                   }
               ]
           }
       }]
   }

Configuration options:

- :code:`path`: Directory on pmem filesystem where data will be stored
- :code:`size`: Maximum size of the pool in bytes (e.g., 10 GB = 10737418240)

In C++ code:

.. code-block:: cpp

   #include <warabi/Provider.hpp>

   auto config = R"(
   {
       "targets": [{
           "type": "pmem",
           "config": {
               "path": "/mnt/pmem/warabi",
               "size": 10737418240
           }
       }]
   }
   )";

   warabi::Provider provider(engine, 42, config);

Pool management
---------------

The pmem backend creates a persistent memory pool at the specified path:

.. code-block:: console

   $ ls -lh /mnt/pmem/warabi/
   -rw-r--r-- 1 user group 10G Dec 31 10:00 pool.pmem

This pool file persists across provider restarts. When the provider starts again
with the same configuration, it opens the existing pool and all regions are
still available.

**Initial creation**: The pool is created on first run:

.. code-block:: console

   [info] [warabi] Creating new pmem pool at /mnt/pmem/warabi/pool.pmem
   [info] [warabi] Pool size: 10 GB

**Subsequent runs**: The pool is opened:

.. code-block:: console

   [info] [warabi] Opening existing pmem pool at /mnt/pmem/warabi/pool.pmem
   [info] [warabi] Recovered 42 existing regions

Region persistence
------------------

Regions created in the pmem backend persist automatically:

.. code-block:: cpp

   // First run: Create and write data
   warabi::RegionID id;
   target.create(&id);
   target.write(id, 0, data.data(), data.size());

   // Provider stops...

   // Second run: Same region ID is still valid
   std::vector<char> buffer(size);
   target.read(id, 0, buffer.data(), buffer.size());  // Data is still there!

**Important**: You must save the region ID somewhere to retrieve it after restart.
The pmem backend doesn't provide a way to enumerate existing regions.

Recommended approach:

.. code-block:: cpp

   // Save region ID to a persistent metadata store (e.g., Yokan)
   metadata_store.put("checkpoint_region", region_id);

   // After restart, retrieve the region ID
   auto region_id = metadata_store.get("checkpoint_region");
   target.read(region_id, 0, buffer, size);  // Data recovered!

Performance characteristics
---------------------------

Pmem offers excellent performance, though slightly slower than pure memory:

**Write latency**: 100-300 nanoseconds (pmem hardware dependent)

**Read latency**: Similar to writes

**Throughput**: GB/s range (depends on pmem hardware and access patterns)

**Sequential vs. random**: Pmem has minimal difference (unlike SSDs)

Benchmark comparison:

.. code-block:: cpp

   // Memory backend: ~10-50 ns latency
   // Pmem backend:   ~100-300 ns latency
   // ABT-IO backend: ~10-100 µs latency (microseconds)

Durability guarantees
---------------------

The pmem backend provides strong durability:

**Synchronous writes**: Data is persistent when :code:`write()` returns

**Power-fail safe**: PMDK ensures consistency across power failures

**Crash recovery**: Provider can recover regions after crashes

Example with crash:

.. code-block:: cpp

   // Before crash
   target.write(region_id, 0, critical_data, size);
   // If crash happens here, data is already persisted

   // After crash and restart
   target.read(region_id, 0, buffer, size);  // Data is intact

Size limits
-----------

Pmem capacity is limited by:

- **Pool size**: Configured at provider startup
- **Device capacity**: Physical pmem hardware size
- **Fragmentation**: Pool may become fragmented over time

Monitor usage:

.. code-block:: cpp

   // Get pool statistics (if Warabi provides this API)
   auto stats = target.getStats();
   std::cout << "Used: " << stats.used_bytes << " bytes\n";
   std::cout << "Free: " << stats.free_bytes << " bytes\n";

Use cases
---------

**1. Checkpointing**: Save application state for fault tolerance

.. code-block:: cpp

   // Checkpoint application state
   auto checkpoint = serialize_state();
   target.write(checkpoint_region, 0, checkpoint.data(), checkpoint.size());

   // After failure, restart from checkpoint
   std::vector<char> saved_state(size);
   target.read(checkpoint_region, 0, saved_state.data(), saved_state.size());
   restore_state(saved_state);

**2. Persistent caching**: Cache data across restarts

.. code-block:: cpp

   // Check if cached data exists
   try {
       size_t size = target.size(cache_region);
       if(size > 0) {
           // Use cached data
           std::vector<char> cached(size);
           target.read(cache_region, 0, cached.data(), size);
           return cached;
       }
   } catch(...) {
       // Cache miss - compute and store
       auto result = expensive_computation();
       target.write(cache_region, 0, result.data(), result.size());
       return result;
   }

**3. Fast persistent storage**: Alternative to databases for structured data

.. code-block:: cpp

   // Store scientific simulation results
   struct SimulationResult {
       double values[1000000];
       int64_t timestamp;
   };

   SimulationResult result = run_simulation();
   target.write(result_region, 0, &result, sizeof(result));

**4. Restart-resilient workflows**: Resume work after interruptions

Cleaning up
-----------

**Destroy individual regions**:

.. code-block:: cpp

   target.destroy(region_id);  // Frees space in pool

**Remove entire pool**:

To completely remove the pool and start fresh:

.. code-block:: console

   # Stop provider first!
   $ rm -rf /mnt/pmem/warabi/

Next provider start will create a new empty pool.

Error handling
--------------

Pmem-specific errors to handle:

**Pool full**:

.. code-block:: cpp

   try {
       target.write(region_id, 0, large_data, size);
   } catch(const warabi::Exception& ex) {
       std::cerr << "Write failed: " << ex.what() << std::endl;
       // Possible: pool is full
       // Solution: Destroy unused regions or increase pool size
   }

**Pool corruption**:

If the pool becomes corrupted (rare), you may need to recreate it:

.. code-block:: console

   $ pmempool check /mnt/pmem/warabi/pool.pmem

Best practices
--------------

**1. Size pools appropriately**: Leave headroom for growth

.. code-block:: json

   {
       "size": 10737418240  // 10 GB for ~8 GB of actual data
   }

**2. Store region IDs persistently**: Keep a catalog in Yokan or similar

**3. Clean up old regions**: Don't let the pool fill up

**4. Monitor pool health**: Check for fragmentation and fullness

**5. Backup critical data**: Pmem is persistent but not a backup solution

**6. Use on DAX-mounted filesystems**: Critical for performance

Troubleshooting
---------------

**Issue**: "Cannot create pmem pool"

**Solution**: Check that path exists and has DAX mount:

.. code-block:: console

   $ mount | grep dax
   /dev/pmem0 on /mnt/pmem type ext4 (rw,dax)

**Issue**: Poor performance

**Solution**: Verify DAX is enabled and check alignment:

.. code-block:: cpp

   // Align writes to 256-byte boundaries for best performance
   size_t alignment = 256;
   size_t aligned_size = ((size + alignment - 1) / alignment) * alignment;

**Issue**: Pool won't open after crash

**Solution**: Use pmempool to repair:

.. code-block:: console

   $ pmempool repair /mnt/pmem/warabi/pool.pmem

Next steps
----------

- :doc:`05_backends_abtio`: Learn about ABT-IO async I/O backend
- :doc:`08_migration`: Migrate data between backends
- :doc:`09_async`: Async operations with pmem
