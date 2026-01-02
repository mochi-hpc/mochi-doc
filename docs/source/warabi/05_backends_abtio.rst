Backend: abt-io
===============

The abt-io backend uses ABT-IO (Argobots-aware I/O) for asynchronous file I/O,
providing persistent storage with non-blocking operations.

When to use
-----------

Use the abt-io backend when:

- You need persistent storage on traditional filesystems
- You want async I/O without blocking Argobots threads
- You're storing very large blobs (multi-GB)
- You don't have persistent memory hardware
- You want compatibility with standard filesystems (ext4, xfs, etc.)

Prerequisites
-------------

The abt-io backend requires ABT-IO:

.. code-block:: console

   # Install ABT-IO
   spack install mochi-abt-io

   # Install Warabi with abt-io support
   spack install mochi-warabi +bedrock +abtio

Characteristics
---------------

**Persistent**: Data stored on disk survives restarts

**Async**: I/O operations don't block Argobots ULTs

**Large capacity**: Limited by filesystem size

**Good throughput**: Optimized for large sequential I/O

**Higher latency**: Microseconds to milliseconds (vs. nanoseconds for memory/pmem)

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
                       "type": "abt-io",
                       "config": {
                           "path": "/tmp/warabi",
                           "num_threads": 4
                       }
                   }
               ]
           }
       }]
   }

Configuration options:

- :code:`path`: Directory where region files will be stored
- :code:`num_threads`: Number of I/O threads for async operations (default: 4)

In C++ code:

.. code-block:: cpp

   #include <warabi/Provider.hpp>

   auto config = R"(
   {
       "targets": [{
           "type": "abt-io",
           "config": {
               "path": "/data/warabi",
               "num_threads": 8
           }
       }]
   }
   )";

   warabi::Provider provider(engine, 42, config);

File organization
-----------------

The abt-io backend creates one file per region:

.. code-block:: console

   $ ls -lh /data/warabi/
   -rw-r--r-- 1 user group  10M Dec 31 10:00 region_0000000000000001.dat
   -rw-r--r-- 1 user group 100M Dec 31 10:01 region_0000000000000002.dat
   -rw-r--r-- 1 user group   1G Dec 31 10:05 region_0000000000000003.dat

Each region is stored in a separate file, named by its region ID.

**Benefits**:
- Easy to inspect and manage
- Can use standard filesystem tools
- Simple backup and recovery

**Note**: Like pmem, you must save region IDs to retrieve data after restart.

Region persistence
------------------

Regions persist across provider restarts:

.. code-block:: cpp

   // First run: Create and write
   warabi::RegionID id;
   target.create(&id);
   std::cout << "Created region: " << id << std::endl;  // Remember this ID!

   target.write(id, 0, data.data(), data.size());

   // Provider stops...

   // Second run: Use saved ID to access data
   warabi::RegionID saved_id = /* retrieve from metadata */;
   std::vector<char> buffer(size);
   target.read(saved_id, 0, buffer.data(), buffer.size());  // Data recovered!

Async I/O behavior
------------------

ABT-IO performs I/O asynchronously using a pool of background threads:

.. code-block:: cpp

   // This doesn't block the current ULT
   target.write(region_id, 0, data, size);

   // The write is queued to I/O threads
   // Current ULT can continue or yield

   // Synchronous completion
   // When write() returns, data is on disk

The number of I/O threads affects concurrency:

**Few threads (1-2)**: Lower overhead, sequential I/O
**Many threads (8-16)**: Higher concurrency, parallel I/O to multiple regions

Performance characteristics
---------------------------

**Sequential writes**: 100s of MB/s to GB/s (depends on storage)
**Random writes**: Lower, depends on filesystem and device
**Large vs. small**: Much better for large operations (>1 MB)

Benchmark example:

.. code-block:: cpp

   const size_t sizes[] = {4096, 65536, 1048576, 10485760};  // 4KB to 10MB

   for(auto size : sizes) {
       std::vector<char> data(size);

       auto start = std::chrono::high_resolution_clock::now();
       target.write(region_id, 0, data.data(), size);
       auto end = std::chrono::high_resolution_clock::now();

       auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
       double throughput = (size / (1024.0 * 1024.0)) / (duration.count() / 1e6);

       std::cout << "Size: " << size << " bytes, Throughput: "
                 << throughput << " MB/s\n";
   }

Typical results:
- Small (4 KB): 10-50 MB/s
- Medium (64 KB): 100-500 MB/s
- Large (1 MB+): 500-2000 MB/s

Use cases
---------

**1. Checkpoint files**: Save large simulation state

.. code-block:: cpp

   // Checkpoint multi-GB state
   std::vector<double> state(100000000);  // 800 MB
   target.write(checkpoint_region, 0, state.data(), state.size() * sizeof(double));

   // Recovery is efficient with abt-io
   target.read(checkpoint_region, 0, state.data(), state.size() * sizeof(double));

**2. Workflow intermediate results**: Store stage outputs

.. code-block:: cpp

   // Stage 1: Write results to disk
   auto results = process_stage1();
   target.write(stage1_region, 0, results.data(), results.size());

   // Stage 2 (possibly on different node): Read results
   std::vector<char> stage1_data(size);
   target.read(stage1_region, 0, stage1_data.data(), stage1_data.size());
   auto final = process_stage2(stage1_data);

**3. Large object storage**: Store datasets too large for memory

.. code-block:: cpp

   // Store a 10 GB dataset
   const size_t dataset_size = 10ULL * 1024 * 1024 * 1024;
   const size_t chunk_size = 100 * 1024 * 1024;  // 100 MB chunks

   for(size_t offset = 0; offset < dataset_size; offset += chunk_size) {
       auto chunk = generate_chunk(offset, chunk_size);
       target.write(dataset_region, offset, chunk.data(), chunk_size);
   }

**4. Archival storage**: Long-term storage of results

Tuning I/O threads
------------------

The number of I/O threads affects performance:

**Rule of thumb**:
- SSDs: 4-8 threads
- HDDs: 1-2 threads per physical disk
- NVMe: 8-16 threads

Example configurations:

.. code-block:: json

   // For single SSD
   {"num_threads": 4}

   // For RAID of SSDs
   {"num_threads": 16}

   // For single HDD
   {"num_threads": 1}

Test different values for your workload:

.. code-block:: cpp

   // Benchmark with different thread counts
   for(int threads : {1, 2, 4, 8, 16}) {
       // Reconfigure and test...
   }

File management
---------------

**Region files persist**: They remain after provider shutdown

**Manual cleanup**:

.. code-block:: console

   $ rm /data/warabi/region_*.dat

**Selective deletion**:

.. code-block:: console

   # Remove specific region
   $ rm /data/warabi/region_0000000000000042.dat

**Backup**:

.. code-block:: console

   # Backup all regions
   $ tar czf warabi-backup.tar.gz /data/warabi/

**Migration**: Can copy files to different storage

Error handling
--------------

ABT-IO specific errors:

**Disk full**:

.. code-block:: cpp

   try {
       target.write(region_id, 0, huge_data, size);
   } catch(const warabi::Exception& ex) {
       std::cerr << "Write failed: " << ex.what() << std::endl;
       // Check: df -h /data
   }

**Permission errors**:

.. code-block:: console

   [error] Cannot create file: Permission denied

   # Fix permissions:
   $ chmod 755 /data/warabi/

**I/O errors**:

.. code-block:: cpp

   // Disk hardware failure, filesystem corruption, etc.
   // Always check return values and handle exceptions

Best practices
--------------

**1. Use for large data**: ABT-IO is optimized for large sequential I/O

.. code-block:: cpp

   // Good: Large writes
   target.write(id, 0, buffer, 10 * 1024 * 1024);  // 10 MB

   // Less efficient: Many small writes
   for(int i = 0; i < 10000; i++) {
       target.write(id, i * 1024, small, 1024);  // 1 KB each
   }

**2. Align I/O**: Use aligned buffers for best performance

.. code-block:: cpp

   // Align to 4 KB page boundary
   void* aligned_buffer;
   posix_memalign(&aligned_buffer, 4096, size);
   target.write(id, 0, aligned_buffer, size);
   free(aligned_buffer);

**3. Clean up unused regions**: Free disk space

.. code-block:: cpp

   target.destroy(unused_region);

**4. Monitor disk space**: Avoid running out

.. code-block:: cpp

   #include <sys/statvfs.h>

   struct statvfs stat;
   statvfs("/data/warabi", &stat);
   uint64_t free_bytes = stat.f_bavail * stat.f_frsize;
   std::cout << "Free: " << free_bytes / (1024*1024*1024) << " GB\n";

**5. Choose appropriate filesystem**: XFS or ext4 recommended

Comparison with other backends
-------------------------------

**vs. memory**:
- ABT-IO: Persistent, slower, larger capacity
- Memory: Volatile, faster, RAM-limited

**vs. pmem**:
- ABT-IO: Standard filesystems, microsecond latency, cheaper
- Pmem: Special hardware, nanosecond latency, expensive

Choose abt-io when:
- You don't have pmem hardware
- You need multi-TB storage
- Microsecond latency is acceptable
- You want standard filesystem compatibility

Next steps
----------

- :doc:`06_transfer_managers`: Optimize data transfer strategies
- :doc:`07_regions`: Advanced region management
- :doc:`08_migration`: Migrate data between backends
- :doc:`09_async`: Advanced async operation patterns
