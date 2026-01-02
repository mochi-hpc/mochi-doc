Async operations
================

Warabi supports asynchronous operations, allowing you to overlap I/O with computation
and issue multiple concurrent operations for better performance.

Why async operations?
---------------------

**Synchronous** (blocking):

.. code-block:: cpp

   // This blocks until complete
   target.write(region_id, 0, data, size);
   // Can't do anything else until write finishes

**Asynchronous** (non-blocking):

.. code-block:: cpp

   // This returns immediately with a request handle
   auto req = target.write_async(region_id, 0, data, size);

   // Do other work while I/O is in progress
   do_other_work();

   // Wait for completion when needed
   req.wait();

Benefits of async:
- **Overlap**: Compute while I/O is in progress
- **Concurrency**: Issue multiple I/O operations simultaneously
- **Better throughput**: Keep storage busy
- **Reduced latency**: Don't wait for each operation

Basic async operations
----------------------

**Async write**:

.. code-block:: cpp

   #include <warabi/Async.hpp>

   // Start async write
   warabi::AsyncRequest write_req = target.write_async(
       region_id, 0, data.data(), data.size()
   );

   // Do other work...
   compute_next_batch();

   // Wait for write to complete
   write_req.wait();

**Async read**:

.. code-block:: cpp

   std::vector<char> buffer(size);

   // Start async read
   warabi::AsyncRequest read_req = target.read_async(
       region_id, 0, buffer.data(), buffer.size()
   );

   // Do other work...
   prepare_for_processing();

   // Wait for read
   read_req.wait();

   // Now buffer contains the data
   process(buffer);

Request handles
---------------

:code:`warabi::AsyncRequest` represents an in-progress operation:

**Check if complete** (non-blocking):

.. code-block:: cpp

   warabi::AsyncRequest req = target.write_async(region_id, 0, data, size);

   while(!req.completed()) {
       do_some_work();
       // Periodically check if done
   }

**Wait for completion** (blocking):

.. code-block:: cpp

   req.wait();  // Blocks until operation completes

**Wait with timeout**:

.. code-block:: cpp

   #include <chrono>

   bool done = req.wait_for(std::chrono::seconds(5));
   if(!done) {
       std::cerr << "Operation timed out!\n";
   }

Multiple concurrent operations
-------------------------------

Issue multiple operations and wait for all:

.. code-block:: cpp

   std::vector<warabi::AsyncRequest> requests;

   // Start multiple writes
   for(size_t i = 0; i < num_regions; i++) {
       auto req = target.write_async(regions[i], 0, data[i], sizes[i]);
       requests.push_back(std::move(req));
   }

   // Wait for all to complete
   for(auto& req : requests) {
       req.wait();
   }

**Wait for any** (first to complete):

.. code-block:: cpp

   std::vector<warabi::AsyncRequest> requests;
   // ... issue operations ...

   while(!requests.empty()) {
       // Wait for first to complete
       size_t idx = warabi::wait_any(requests);

       std::cout << "Request " << idx << " completed\n";
       process_result(idx);

       // Remove completed request
       requests.erase(requests.begin() + idx);
   }

Pipeline pattern
----------------

Overlap I/O and computation:

.. code-block:: cpp

   const size_t PIPELINE_DEPTH = 4;
   std::queue<warabi::AsyncRequest> in_flight;

   for(size_t i = 0; i < total_batches; i++) {
       // Wait if pipeline is full
       if(in_flight.size() >= PIPELINE_DEPTH) {
           in_flight.front().wait();
           in_flight.pop();
       }

       // Generate next batch
       auto batch = generate_batch(i);

       // Issue async write
       auto req = target.write_async(batch.region, 0, batch.data, batch.size);
       in_flight.push(std::move(req));
   }

   // Drain remaining operations
   while(!in_flight.empty()) {
       in_flight.front().wait();
       in_flight.pop();
   }

Double buffering
----------------

Use double buffering to keep both CPU and I/O busy:

.. code-block:: cpp

   std::vector<char> buffer1(size);
   std::vector<char> buffer2(size);

   // Read first chunk
   target.read(region_id, 0, buffer1.data(), size);

   for(size_t offset = size; offset < total_size; offset += size) {
       // Start reading next chunk asynchronously
       auto read_req = target.read_async(
           region_id, offset, buffer2.data(), size
       );

       // Process current chunk while next is loading
       process(buffer1);

       // Wait for next chunk
       read_req.wait();

       // Swap buffers
       std::swap(buffer1, buffer2);
   }

   // Process last chunk
   process(buffer1);

Scatter-gather I/O
------------------

Read/write multiple regions in parallel:

.. code-block:: cpp

   struct ScatterGatherOp {
       warabi::RegionID region;
       size_t offset;
       void* buffer;
       size_t size;
   };

   void scatter_write(
       warabi::TargetHandle& target,
       const std::vector<ScatterGatherOp>& ops)
   {
       std::vector<warabi::AsyncRequest> requests;

       // Issue all writes
       for(const auto& op : ops) {
           auto req = target.write_async(
               op.region, op.offset, op.buffer, op.size
           );
           requests.push_back(std::move(req));
       }

       // Wait for all
       for(auto& req : requests) {
           req.wait();
       }
   }

   void gather_read(
       warabi::TargetHandle& target,
       const std::vector<ScatterGatherOp>& ops)
   {
       std::vector<warabi::AsyncRequest> requests;

       // Issue all reads
       for(const auto& op : ops) {
           auto req = target.read_async(
               op.region, op.offset, op.buffer, op.size
           );
           requests.push_back(std::move(req));
       }

       // Wait for all
       for(auto& req : requests) {
           req.wait();
       }
   }

Async with Argobots
--------------------

Warabi's async operations integrate with Argobots:

.. code-block:: cpp

   #include <abt.h>

   void ult_function(void* args) {
       warabi::TargetHandle* target = (warabi::TargetHandle*)args;

       // Async operation in ULT
       auto req = target->write_async(region_id, 0, data, size);

       // Yield while waiting (allows other ULTs to run)
       while(!req.completed()) {
           ABT_thread_yield();
       }

       req.wait();  // Should return immediately since it's complete
   }

   // Create ULT
   ABT_thread ult;
   ABT_thread_create(pool, ult_function, &target, ABT_THREAD_ATTR_NULL, &ult);

Error handling
--------------

Check for errors after async operations:

.. code-block:: cpp

   auto req = target.write_async(region_id, 0, data, size);

   // ... do other work ...

   req.wait();

   // Check for errors
   if(req.error()) {
       std::cerr << "Async write failed: " << req.error_message() << "\n";
       // Handle error
   }

**Exceptions**:

Some implementations may throw exceptions:

.. code-block:: cpp

   try {
       auto req = target.write_async(region_id, 0, data, size);
       req.wait();
   } catch(const warabi::Exception& ex) {
       std::cerr << "Async operation failed: " << ex.what() << "\n";
   }

Performance comparison
----------------------

Benchmark sync vs. async:

.. code-block:: cpp

   const size_t NUM_OPERATIONS = 100;
   std::vector<warabi::RegionID> regions(NUM_OPERATIONS);

   // Synchronous
   auto start = std::chrono::high_resolution_clock::now();
   for(size_t i = 0; i < NUM_OPERATIONS; i++) {
       target.write(regions[i], 0, data, size);
   }
   auto sync_duration = std::chrono::high_resolution_clock::now() - start;

   // Asynchronous
   start = std::chrono::high_resolution_clock::now();
   std::vector<warabi::AsyncRequest> requests;
   for(size_t i = 0; i < NUM_OPERATIONS; i++) {
       requests.push_back(target.write_async(regions[i], 0, data, size));
   }
   for(auto& req : requests) {
       req.wait();
   }
   auto async_duration = std::chrono::high_resolution_clock::now() - start;

   std::cout << "Sync:  " << sync_duration.count() << " ms\n";
   std::cout << "Async: " << async_duration.count() << " ms\n";
   std::cout << "Speedup: " << (double)sync_duration.count() / async_duration.count() << "x\n";

Typical speedup: 2-10x for many small operations.

Use cases
---------

**Use case 1: Batch checkpointing**

Checkpoint multiple components in parallel:

.. code-block:: cpp

   struct Component {
       std::vector<char> state;
       warabi::RegionID region;
   };

   std::vector<Component> components = /* ... */;

   // Start all checkpoints in parallel
   std::vector<warabi::AsyncRequest> requests;
   for(auto& comp : components) {
       auto req = target.write_async(
           comp.region, 0, comp.state.data(), comp.state.size()
       );
       requests.push_back(std::move(req));
   }

   // Wait for all
   for(auto& req : requests) {
       req.wait();
   }

**Use case 2: Prefetching**

Load data before it's needed:

.. code-block:: cpp

   // Prefetch next region
   std::vector<char> prefetch_buffer(size);
   auto prefetch_req = target.read_async(
       next_region, 0, prefetch_buffer.data(), size
   );

   // Process current region
   process(current_buffer);

   // Wait for prefetch
   prefetch_req.wait();
   current_buffer = std::move(prefetch_buffer);

**Use case 3: Parallel data loading**

Load dataset shards in parallel:

.. code-block:: cpp

   const size_t NUM_SHARDS = 10;
   std::vector<warabi::RegionID> shards(NUM_SHARDS);
   std::vector<std::vector<char>> shard_data(NUM_SHARDS);

   std::vector<warabi::AsyncRequest> requests;
   for(size_t i = 0; i < NUM_SHARDS; i++) {
       shard_data[i].resize(shard_size);
       auto req = target.read_async(
           shards[i], 0, shard_data[i].data(), shard_size
       );
       requests.push_back(std::move(req));
   }

   // Wait for all shards
   for(auto& req : requests) {
       req.wait();
   }

   // Now all shards are loaded
   auto combined = combine_shards(shard_data);

Best practices
--------------

**1. Issue operations early**: Start async operations as early as possible

.. code-block:: cpp

   // Good
   auto req = target.read_async(region_id, 0, buffer, size);
   do_prep_work();
   req.wait();

   // Less optimal
   do_prep_work();
   auto req = target.read_async(region_id, 0, buffer, size);
   req.wait();  // No overlap

**2. Limit concurrency**: Don't issue unlimited concurrent operations

.. code-block:: cpp

   const size_t MAX_CONCURRENT = 16;

   for(size_t i = 0; i < total_operations; i++) {
       if(in_flight.size() >= MAX_CONCURRENT) {
           in_flight.front().wait();
           in_flight.pop();
       }

       auto req = target.write_async(regions[i], 0, data[i], sizes[i]);
       in_flight.push(std::move(req));
   }

**3. Check for errors**: Always check async operation results

**4. Clean up**: Ensure all requests complete before exit

.. code-block:: cpp

   {
       std::vector<warabi::AsyncRequest> requests;
       // ... issue operations ...

       // Ensure all complete before going out of scope
       for(auto& req : requests) {
           req.wait();
       }
   }  // Safe to destroy

**5. Profile**: Measure to verify async actually helps your workload

Next steps
----------

- :doc:`10_bedrock`: Configure for optimal async performance
- :doc:`06_transfer_managers`: Combine async with pipeline transfers
- :ref:`Argobots`: Learn more about Argobots integration
