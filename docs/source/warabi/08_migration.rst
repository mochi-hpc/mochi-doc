Data migration
==============

Warabi supports migrating regions between targets and providers, allowing you to:
- Move data between backends (e.g., memory → pmem)
- Rebalance data across providers
- Migrate data to different hardware
- Upgrade or reconfigure storage

Migration basics
----------------

Migration copies a region from one target to another:

.. code-block:: cpp

   // Source target (memory backend)
   warabi::TargetHandle source = client.makeTargetHandle(addr1, pid1, 0);

   // Destination target (pmem backend)
   warabi::TargetHandle dest = client.makeTargetHandle(addr2, pid2, 0);

   // Create region in source
   warabi::RegionID source_region;
   source.create(&source_region);
   source.write(source_region, 0, data, size);

   // Migrate to destination
   warabi::RegionID dest_region;
   source.migrate(source_region, dest, &dest_region);

   // Now data exists in both source and dest
   // Optionally destroy source region
   source.destroy(source_region);

After migration, you have two independent regions. The source region is not
automatically deleted - you must explicitly destroy it if desired.

Migration scenarios
-------------------

**Scenario 1: Backend migration**

Move from volatile to persistent storage:

.. code-block:: cpp

   // Initially in memory for fast writes
   warabi::TargetHandle memory_target = /* ... */;
   warabi::RegionID mem_region;
   memory_target.create(&mem_region);

   // Write data quickly
   memory_target.write(mem_region, 0, data, size);

   // Migrate to persistent storage
   warabi::TargetHandle pmem_target = /* ... */;
   warabi::RegionID persistent_region;
   memory_target.migrate(mem_region, pmem_target, &persistent_region);

   // Clean up memory region
   memory_target.destroy(mem_region);

**Scenario 2: Load balancing**

Distribute data across multiple providers:

.. code-block:: cpp

   std::vector<warabi::TargetHandle> targets = {target1, target2, target3};

   // Initially all data on target1
   for(auto& region : regions) {
       // Migrate some regions to other targets
       size_t dest_idx = hash(region) % targets.size();
       if(dest_idx != 0) {
           warabi::RegionID new_region;
           target1.migrate(region, targets[dest_idx], &new_region);
           // Update region mapping
           region_map[region_name] = new_region;
           target1.destroy(region);
       }
   }

**Scenario 3: Archival**

Move old data to slower, cheaper storage:

.. code-block:: cpp

   // Hot data on fast storage
   warabi::TargetHandle fast = client.makeTargetHandle(fast_addr, pid, 0);

   // Cold data on slow storage
   warabi::TargetHandle slow = client.makeTargetHandle(slow_addr, pid, 1);

   // Archive old regions
   if(is_old(region_id)) {
       warabi::RegionID archived;
       fast.migrate(region_id, slow, &archived);
       fast.destroy(region_id);

       // Update catalog
       catalog.update(region_name, archived, slow_addr);
   }

Migration performance
---------------------

Migration performance depends on:

- **Data size**: Larger regions take longer
- **Network bandwidth**: Between source and destination
- **Backend speeds**: Read from source, write to destination
- **Transfer manager**: Pipeline can help for large migrations

**Optimizing migration**:

Configure the destination provider with pipeline transfer manager:

.. code-block:: json

   {
       "providers": [{
           "name": "migration_dest",
           "provider_id": 2,
           "config": {
               "targets": [{"type": "pmem", "config": {...}}],
               "transfer_manager": {
                   "type": "pipeline",
                   "config": {
                       "num_threads": 8,
                       "pipeline_size": 16777216
                   }
               }
           }
       }]
   }

**Monitoring migration**:

.. code-block:: cpp

   auto start = std::chrono::high_resolution_clock::now();

   warabi::RegionID dest_region;
   source.migrate(source_region, dest, &dest_region);

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

   size_t region_size = source.size(source_region);
   double throughput = (region_size / (1024.0 * 1024.0)) / duration.count();

   std::cout << "Migrated " << region_size / (1024*1024) << " MB in "
             << duration.count() << "s (" << throughput << " MB/s)\n";

Bulk migration
--------------

Migrate multiple regions efficiently:

.. code-block:: cpp

   std::vector<warabi::RegionID> source_regions = /* ... */;
   std::vector<warabi::RegionID> dest_regions(source_regions.size());

   // Sequential migration
   for(size_t i = 0; i < source_regions.size(); i++) {
       source.migrate(source_regions[i], dest, &dest_regions[i]);
   }

   // Parallel migration (if provider supports concurrent operations)
   std::vector<std::thread> threads;
   for(size_t i = 0; i < source_regions.size(); i++) {
       threads.emplace_back([&, i]() {
           source.migrate(source_regions[i], dest, &dest_regions[i]);
       });
   }
   for(auto& t : threads) t.join();

Online migration
----------------

Migrate without downtime by using a copy-on-write approach:

.. code-block:: cpp

   // Phase 1: Start migration (async if available)
   warabi::RegionID dest_region;
   source.migrate(source_region, dest, &dest_region);

   // Phase 2: Record new writes during migration
   std::vector<Write> writes_during_migration;

   // Phase 3: Replay writes to destination
   for(const auto& write : writes_during_migration) {
       dest.write(dest_region, write.offset, write.data, write.size);
   }

   // Phase 4: Switch to destination
   current_region = dest_region;
   current_target = dest;

   // Phase 5: Clean up source
   source.destroy(source_region);

This ensures no data loss during migration.

Migration policies
------------------

**Policy 1: Tiered storage**

Automatically move data based on access patterns:

.. code-block:: cpp

   struct RegionAccessInfo {
       warabi::RegionID id;
       size_t access_count;
       std::chrono::steady_clock::time_point last_access;
   };

   void apply_tiering_policy(
       std::vector<RegionAccessInfo>& regions,
       warabi::TargetHandle& hot_tier,
       warabi::TargetHandle& cold_tier)
   {
       auto now = std::chrono::steady_clock::now();

       for(auto& info : regions) {
           auto age = std::chrono::duration_cast<std::chrono::hours>(
               now - info.last_access).count();

           // Move to cold tier if not accessed in 24 hours
           if(age > 24 && info.access_count < 10) {
               warabi::RegionID cold_region;
               hot_tier.migrate(info.id, cold_tier, &cold_region);
               hot_tier.destroy(info.id);

               info.id = cold_region;
               std::cout << "Moved region to cold tier\n";
           }
       }
   }

**Policy 2: Capacity-based migration**

Rebalance when a target fills up:

.. code-block:: cpp

   void balance_targets(std::vector<warabi::TargetHandle>& targets) {
       // Get usage for each target
       std::vector<size_t> usage(targets.size());
       for(size_t i = 0; i < targets.size(); i++) {
           usage[i] = get_target_usage(targets[i]);
       }

       // Find most and least loaded
       size_t max_idx = std::max_element(usage.begin(), usage.end()) - usage.begin();
       size_t min_idx = std::min_element(usage.begin(), usage.end()) - usage.begin();

       // Migrate some regions from max to min
       if(usage[max_idx] - usage[min_idx] > threshold) {
           auto regions_to_move = select_regions_to_migrate(targets[max_idx]);
           for(auto& region : regions_to_move) {
               warabi::RegionID new_region;
               targets[max_idx].migrate(region, targets[min_idx], &new_region);
               targets[max_idx].destroy(region);
           }
       }
   }

Error handling
--------------

Handle migration failures gracefully:

.. code-block:: cpp

   try {
       warabi::RegionID dest_region;
       source.migrate(source_region, dest, &dest_region);

       // Verify migration success
       size_t source_size = source.size(source_region);
       size_t dest_size = dest.size(dest_region);

       if(source_size != dest_size) {
           std::cerr << "Migration size mismatch!\n";
           dest.destroy(dest_region);  // Clean up failed migration
           throw std::runtime_error("Migration verification failed");
       }

       // Safe to destroy source now
       source.destroy(source_region);

   } catch(const warabi::Exception& ex) {
       std::cerr << "Migration failed: " << ex.what() << "\n";
       // Source region still intact, can retry
   }

Migration with REMI
-------------------

Warabi can integrate with REMI (Remote Memory Integration) for more advanced
migration features. REMI provides:

- Pre-copy migration
- Post-copy migration
- Live migration
- Incremental migration

See REMI documentation for details.

Best practices
--------------

**1. Verify before cleanup**:

.. code-block:: cpp

   // Migrate
   warabi::RegionID dest_region;
   source.migrate(source_region, dest, &dest_region);

   // Verify
   if(verify_migration(source, source_region, dest, dest_region)) {
       source.destroy(source_region);  // Safe to clean up
   }

**2. Update metadata**:

.. code-block:: cpp

   // After migration, update region catalog
   catalog.update_location(region_name, dest_region, dest_address);

**3. Plan for failures**:

Always have a rollback plan if migration fails:

.. code-block:: cpp

   // Keep source until migration confirmed
   // Only destroy source after successful verification

**4. Test migrations**:

Test migration before production deployment:

.. code-block:: cpp

   // Test with small regions first
   warabi::RegionID test_region;
   source.create(&test_region);
   source.write(test_region, 0, test_data, 1024);

   warabi::RegionID migrated;
   source.migrate(test_region, dest, &migrated);

   // Verify and measure
   verify_and_benchmark_migration();

**5. Monitor progress**:

For large migrations, monitor progress:

.. code-block:: cpp

   // For very large regions, consider chunked manual migration
   const size_t CHUNK_SIZE = 100 * 1024 * 1024;  // 100 MB
   size_t region_size = source.size(source_region);

   warabi::RegionID dest_region;
   dest.create(&dest_region);

   for(size_t offset = 0; offset < region_size; offset += CHUNK_SIZE) {
       size_t chunk = std::min(CHUNK_SIZE, region_size - offset);

       std::vector<char> buffer(chunk);
       source.read(source_region, offset, buffer.data(), chunk);
       dest.write(dest_region, offset, buffer.data(), chunk);

       double progress = 100.0 * (offset + chunk) / region_size;
       std::cout << "Migration progress: " << progress << "%\n";
   }

Next steps
----------

- :doc:`09_async`: Async migration for better performance
- :doc:`10_bedrock`: Configure migration-friendly deployments
- :ref:`REMI`: Advanced migration with REMI
