Transfer managers
=================

Transfer managers control how data is transferred between clients and Warabi providers.
They allow you to optimize for different scenarios: single large transfers, pipelined transfers, or concurrent transfers.

What are transfer managers?
----------------------------

A transfer manager is a strategy for moving data between client and server. Different
strategies offer different trade-offs between throughput, latency, and resource usage.

Warabi provides two built-in transfer managers:

- **Default**: Simple single-transfer strategy
- **Pipeline**: Concurrent pipelined transfers for better throughput

When to use each
----------------

**Default transfer manager**:
- Simple use cases
- Small to medium data sizes (< 10 MB)
- When latency is more important than throughput
- Lower memory overhead

**Pipeline transfer manager**:
- Large data transfers (> 10 MB)
- When maximizing throughput is critical
- Multiple concurrent regions
- When you have bandwidth to spare

Configuration
-------------

Transfer managers are configured at the provider level:

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
               ],
               "transfer_manager": {
                   "type": "pipeline",
                   "config": {
                       "num_threads": 4,
                       "pipeline_size": 8388608
                   }
               }
           }
       }]
   }

In C++ code:

.. code-block:: cpp

   auto config = R"(
   {
       "targets": [{"type": "memory", "config": {}}],
       "transfer_manager": {
           "type": "pipeline",
           "config": {
               "num_threads": 4,
               "pipeline_size": 8388608
           }
       }
   }
   )";

   warabi::Provider provider(engine, 42, config);

Default transfer manager
-------------------------

The default transfer manager performs simple single-buffer transfers.

**Configuration**: No configuration needed (used when transfer_manager is not specified)

.. code-block:: json

   {
       "config": {
           "targets": [{"type": "memory", "config": {}}]
           // No transfer_manager specified = use default
       }
   }

**Behavior**:
- Single bulk transfer per operation
- Straightforward memory management
- Good for most use cases

**Performance**: Fine for small to medium transfers, may not saturate bandwidth
for very large transfers.

Pipeline transfer manager
-------------------------

The pipeline transfer manager splits large transfers into chunks and processes
them concurrently.

**Configuration options**:

.. code-block:: json

   {
       "transfer_manager": {
           "type": "pipeline",
           "config": {
               "num_threads": 4,
               "pipeline_size": 8388608
           }
       }
   }

- :code:`num_threads`: Number of concurrent pipeline stages (default: 4)
- :code:`pipeline_size`: Size of each pipeline chunk in bytes (default: 8 MB)

**How it works**:

For a large write operation:

1. Data is split into chunks of :code:`pipeline_size` bytes
2. Up to :code:`num_threads` chunks are transferred concurrently
3. As chunks complete, new ones start
4. Process continues until all data is transferred

Example:

.. code-block:: text

   100 MB write with pipeline_size=10MB, num_threads=4:

   Time ------>
   Thread 1: [Chunk 1] [Chunk 5] [Chunk 9]
   Thread 2: [Chunk 2] [Chunk 6] [Chunk 10]
   Thread 3: [Chunk 3] [Chunk 7]
   Thread 4: [Chunk 4] [Chunk 8]

Performance comparison
----------------------

Benchmark comparing default vs. pipeline:

.. code-block:: cpp

   const size_t data_size = 100 * 1024 * 1024;  // 100 MB
   std::vector<char> data(data_size);

   // With default transfer manager
   auto start = std::chrono::high_resolution_clock::now();
   target.write(region_id, 0, data.data(), data_size);
   auto end = std::chrono::high_resolution_clock::now();

   auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
   double throughput = (data_size / (1024.0 * 1024.0)) / (duration.count() / 1000.0);

   std::cout << "Default: " << throughput << " MB/s\n";

   // With pipeline transfer manager (reconfigure provider first)
   // ... repeat test ...
   std::cout << "Pipeline: " << pipeline_throughput << " MB/s\n";

Typical results (10 GbE network):
- Default: 400-600 MB/s
- Pipeline (4 threads): 800-1000 MB/s

Tuning pipeline parameters
---------------------------

**Number of threads**:

More threads = higher concurrency, but diminishing returns:

.. code-block:: json

   // Conservative (lower overhead)
   {"num_threads": 2}

   // Balanced
   {"num_threads": 4}

   // Aggressive (for very fast networks)
   {"num_threads": 8}

**Pipeline size**:

Larger chunks = fewer pipeline stages, but higher per-transfer latency:

.. code-block:: json

   // Small chunks (good for slow networks)
   {"pipeline_size": 1048576}  // 1 MB

   // Medium chunks (balanced)
   {"pipeline_size": 8388608}  // 8 MB

   // Large chunks (fast networks/storage)
   {"pipeline_size": 33554432}  // 32 MB

**Rule of thumb**:
- Fast network (>10 GbE): Larger chunks, more threads
- Slow network (<1 GbE): Smaller chunks, fewer threads
- Balance :code:`num_threads` × :code:`pipeline_size` ≈ total transfer size

Use cases
---------

**Use default for**:

.. code-block:: cpp

   // Small frequent writes
   for(int i = 0; i < 1000; i++) {
       std::vector<char> small_data(4096);
       target.write(regions[i], 0, small_data.data(), small_data.size());
   }

**Use pipeline for**:

.. code-block:: cpp

   // Large checkpoint (multi-GB)
   std::vector<char> checkpoint(5ULL * 1024 * 1024 * 1024);  // 5 GB
   target.write(checkpoint_region, 0, checkpoint.data(), checkpoint.size());

   // Bulk data loading
   std::vector<char> dataset(50ULL * 1024 * 1024 * 1024);  // 50 GB
   target.write(dataset_region, 0, dataset.data(), dataset.size());

Memory considerations
---------------------

Pipeline manager uses more memory:

.. code-block:: text

   Memory per transfer = num_threads × pipeline_size

Example:
- num_threads=4, pipeline_size=8 MB → 32 MB per transfer
- Multiple concurrent transfers → multiply by number of regions

If memory is constrained:

.. code-block:: json

   {
       "num_threads": 2,
       "pipeline_size": 4194304  // 2 × 4MB = 8MB total
   }

Client-side considerations
--------------------------

Transfer manager is configured server-side, but affects client behavior:

**Buffer alignment**: For best performance, align buffers:

.. code-block:: cpp

   void* aligned_buffer;
   posix_memalign(&aligned_buffer, 4096, size);
   target.write(region_id, 0, aligned_buffer, size);
   free(aligned_buffer);

**Large buffers**: Ensure client has memory for the full transfer:

.. code-block:: cpp

   // This requires the full buffer in client memory
   std::vector<char> huge_data(10ULL * 1024 * 1024 * 1024);  // 10 GB
   target.write(region_id, 0, huge_data.data(), huge_data.size());

Monitoring and debugging
------------------------

**Track transfer performance**:

.. code-block:: cpp

   auto start = std::chrono::high_resolution_clock::now();
   target.write(region_id, 0, data, size);
   auto end = std::chrono::high_resolution_clock::now();

   auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
   double mb_per_sec = (size / (1024.0 * 1024.0)) / (ms / 1000.0);

   if(mb_per_sec < expected_throughput) {
       std::cerr << "Slow transfer: " << mb_per_sec << " MB/s\n";
       // Consider adjusting pipeline parameters
   }

**Network bandwidth utilization**:

.. code-block:: console

   # Monitor network during transfer
   $ iftop
   $ nload

If bandwidth is not saturated, increase :code:`num_threads` or :code:`pipeline_size`.

Best practices
--------------

**1. Match to workload**:
- Small/frequent → default
- Large/bulk → pipeline

**2. Tune for your network**:
- Test different configurations
- Monitor bandwidth utilization

**3. Consider memory**:
- Don't over-allocate pipeline buffers
- Balance parallelism vs. memory

**4. Benchmark**:
- Always measure performance
- Don't assume pipeline is always better

**5. Consistency**:
- Use same configuration across all providers in a deployment

Example: Adaptive configuration
--------------------------------

Different providers for different use cases:

.. code-block:: json

   {
       "providers": [
           {
               "name": "small_objects",
               "provider_id": 1,
               "config": {
                   "targets": [{"type": "memory", "config": {}}]
                   // Uses default transfer manager
               }
           },
           {
               "name": "large_objects",
               "provider_id": 2,
               "config": {
                   "targets": [{"type": "abt-io", "config": {...}}],
                   "transfer_manager": {
                       "type": "pipeline",
                       "config": {
                           "num_threads": 8,
                           "pipeline_size": 16777216
                       }
                   }
               }
           }
       ]
   }

Clients choose the appropriate provider based on data size.

Next steps
----------

- :doc:`07_regions`: Advanced region management techniques
- :doc:`08_migration`: Migrate data with optimal transfer settings
- :doc:`09_async`: Async operations for overlapping I/O
