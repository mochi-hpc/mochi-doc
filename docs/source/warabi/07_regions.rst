Advanced region management
==========================

This tutorial covers advanced techniques for managing Warabi regions, including
region metadata, lifecycle management, and best practices for production deployments.

Region lifecycle
----------------

Understanding the complete lifecycle of a region:

.. code-block:: cpp

   // 1. Creation
   warabi::RegionID id;
   target.create(&id);  // Region exists but is empty

   // 2. Growth (implicit)
   target.write(id, 0, data1, 1024);     // Region size: 1 KB
   target.write(id, 1024, data2, 1024);  // Region size: 2 KB

   // 3. Usage
   target.read(id, 0, buffer, 2048);     // Read all data

   // 4. Destruction
   target.destroy(id);  // Region removed, ID invalid

**Important**: Region IDs become invalid after destruction. Reusing a destroyed
ID results in errors.

Region metadata tracking
------------------------

Warabi doesn't provide built-in metadata for regions. For production use, you
should maintain metadata externally:

.. code-block:: cpp

   #include <map>
   #include <string>

   struct RegionMetadata {
       warabi::RegionID id;
       std::string name;
       std::string description;
       time_t created;
       size_t expected_size;
       std::string owner;
   };

   class RegionCatalog {
       std::map<std::string, RegionMetadata> catalog;

   public:
       void registerRegion(const std::string& name,
                          warabi::RegionID id,
                          const std::string& description) {
           RegionMetadata meta;
           meta.id = id;
           meta.name = name;
           meta.description = description;
           meta.created = time(nullptr);
           catalog[name] = meta;
       }

       warabi::RegionID lookup(const std::string& name) {
           return catalog[name].id;
       }

       void remove(const std::string& name) {
           catalog.erase(name);
       }
   };

**Integration with Yokan**:

For distributed deployments, store metadata in Yokan:

.. code-block:: cpp

   #include <yokan/database.hpp>

   // Store region mapping
   yk_database_handle_t db = /* ... */;

   // Register region
   std::string key = "region:" + region_name;
   yk_put(db, key.data(), key.size(),
          &region_id, sizeof(region_id), YOKAN_MODE_DEFAULT);

   // Lookup region
   warabi::RegionID id;
   yk_get(db, key.data(), key.size(), &id, &sizeof(region_id));

Region size management
----------------------

**Query current size**:

.. code-block:: cpp

   size_t current_size = target.size(region_id);
   std::cout << "Region contains " << current_size << " bytes\n";

**Pre-sizing regions**:

Some backends benefit from knowing the expected size upfront:

.. code-block:: cpp

   warabi::RegionID id;
   target.create(&id);

   // Write at the end to pre-allocate
   char dummy = 0;
   size_t expected_size = 1024 * 1024 * 100;  // 100 MB
   target.write(id, expected_size - 1, &dummy, 1);

   // Now write actual data
   target.write(id, 0, data, data_size);

This can improve performance for backends like abt-io by pre-allocating disk space.

Multi-region workflows
----------------------

**Pattern 1: Checkpoint sets**

Create related regions for multi-component state:

.. code-block:: cpp

   struct CheckpointSet {
       warabi::RegionID state_region;
       warabi::RegionID metadata_region;
       warabi::RegionID index_region;
   };

   CheckpointSet create_checkpoint(warabi::TargetHandle& target) {
       CheckpointSet cp;

       target.create(&cp.state_region);
       target.create(&cp.metadata_region);
       target.create(&cp.index_region);

       // Write components
       target.write(cp.state_region, 0, state_data, state_size);
       target.write(cp.metadata_region, 0, metadata, metadata_size);
       target.write(cp.index_region, 0, index, index_size);

       return cp;
   }

**Pattern 2: Versioned regions**

Maintain multiple versions of data:

.. code-block:: cpp

   std::vector<warabi::RegionID> versions;

   // Create new version
   warabi::RegionID new_version;
   target.create(&new_version);
   target.write(new_version, 0, updated_data, size);
   versions.push_back(new_version);

   // Keep last N versions
   const int MAX_VERSIONS = 5;
   while(versions.size() > MAX_VERSIONS) {
       target.destroy(versions.front());
       versions.erase(versions.begin());
   }

**Pattern 3: Sharded data**

Split large datasets across multiple regions:

.. code-block:: cpp

   const size_t SHARD_SIZE = 100 * 1024 * 1024;  // 100 MB per shard
   std::vector<warabi::RegionID> shards;

   size_t total_size = dataset.size();
   size_t num_shards = (total_size + SHARD_SIZE - 1) / SHARD_SIZE;

   for(size_t i = 0; i < num_shards; i++) {
       warabi::RegionID shard;
       target.create(&shard);

       size_t offset = i * SHARD_SIZE;
       size_t shard_size = std::min(SHARD_SIZE, total_size - offset);

       target.write(shard, 0, dataset.data() + offset, shard_size);
       shards.push_back(shard);
   }

   // Later: Read shards in parallel
   std::vector<std::thread> threads;
   for(size_t i = 0; i < shards.size(); i++) {
       threads.emplace_back([&, i]() {
           std::vector<char> shard_data(SHARD_SIZE);
           target.read(shards[i], 0, shard_data.data(), SHARD_SIZE);
           process_shard(i, shard_data);
       });
   }
   for(auto& t : threads) t.join();

Resource limits
---------------

**Per-target limits**:

Different backends have different limits:

- **memory**: Limited by RAM
- **pmem**: Limited by pool size
- **abt-io**: Limited by filesystem

**Checking available space**:

.. code-block:: cpp

   // For filesystem-based backends (abt-io)
   #include <sys/statvfs.h>

   bool check_space(const std::string& path, size_t required) {
       struct statvfs stat;
       if(statvfs(path.c_str(), &stat) != 0) return false;

       uint64_t available = stat.f_bavail * stat.f_bsize;
       return available >= required;
   }

   // Before creating large region
   if(!check_space("/data/warabi", 10ULL * 1024 * 1024 * 1024)) {
       std::cerr << "Insufficient space!\n";
       return -1;
   }

Region cleanup strategies
-------------------------

**Strategy 1: Explicit cleanup**

Clean up regions immediately when done:

.. code-block:: cpp

   {
       warabi::RegionID temp;
       target.create(&temp);

       // Use region...
       target.write(temp, 0, data, size);

       // Clean up immediately
       target.destroy(temp);
   }  // temp goes out of scope

**Strategy 2: Lazy cleanup**

Batch cleanup operations:

.. code-block:: cpp

   std::vector<warabi::RegionID> to_cleanup;

   // Mark for cleanup
   to_cleanup.push_back(old_region);

   // Later: Batch cleanup
   for(auto& id : to_cleanup) {
       try {
           target.destroy(id);
       } catch(const warabi::Exception& ex) {
           // Log but continue
           std::cerr << "Failed to destroy region: " << ex.what() << "\n";
       }
   }
   to_cleanup.clear();

**Strategy 3: Reference counting**

Track region usage:

.. code-block:: cpp

   std::map<warabi::RegionID, int> ref_counts;

   void acquire_region(warabi::RegionID id) {
       ref_counts[id]++;
   }

   void release_region(warabi::RegionID id, warabi::TargetHandle& target) {
       if(--ref_counts[id] == 0) {
           target.destroy(id);
           ref_counts.erase(id);
       }
   }

Error recovery
--------------

**Handling invalid regions**:

.. code-block:: cpp

   bool region_exists(warabi::TargetHandle& target, warabi::RegionID id) {
       try {
           target.size(id);  // Will throw if region doesn't exist
           return true;
       } catch(const warabi::Exception&) {
           return false;
       }
   }

   // Use in recovery
   if(!region_exists(target, saved_region_id)) {
       std::cerr << "Region not found, recreating...\n";
       target.create(&saved_region_id);
   }

**Graceful degradation**:

.. code-block:: cpp

   try {
       target.read(primary_region, 0, buffer, size);
   } catch(const warabi::Exception& ex) {
       std::cerr << "Primary failed: " << ex.what() << "\n";

       // Try backup region
       try {
           target.read(backup_region, 0, buffer, size);
       } catch(const warabi::Exception& ex2) {
           std::cerr << "Backup also failed: " << ex2.what() << "\n";
           // Regenerate data
           regenerate_data(buffer, size);
       }
   }

Monitoring and diagnostics
---------------------------

**Track region statistics**:

.. code-block:: cpp

   struct RegionStats {
       size_t total_reads = 0;
       size_t total_writes = 0;
       size_t bytes_read = 0;
       size_t bytes_written = 0;
       std::chrono::steady_clock::time_point last_access;
   };

   std::map<warabi::RegionID, RegionStats> stats;

   void tracked_write(warabi::TargetHandle& target, warabi::RegionID id,
                      size_t offset, const void* data, size_t size) {
       target.write(id, offset, data, size);

       stats[id].total_writes++;
       stats[id].bytes_written += size;
       stats[id].last_access = std::chrono::steady_clock::now();
   }

**Periodic reporting**:

.. code-block:: cpp

   void print_stats() {
       std::cout << "Region Statistics:\n";
       for(const auto& [id, stat] : stats) {
           std::cout << "  Region " << id << ":\n";
           std::cout << "    Reads: " << stat.total_reads << "\n";
           std::cout << "    Writes: " << stat.total_writes << "\n";
           std::cout << "    Data read: " << stat.bytes_read / (1024*1024) << " MB\n";
           std::cout << "    Data written: " << stat.bytes_written / (1024*1024) << " MB\n";
       }
   }

Best practices summary
----------------------

**1. Maintain external metadata**: Track region names, ownership, creation time

**2. Clean up promptly**: Destroy regions when no longer needed

**3. Handle errors gracefully**: Always check for invalid regions

**4. Monitor usage**: Track statistics for optimization

**5. Use appropriate patterns**: Choose sharding, versioning, or checkpointing as needed

**6. Test recovery**: Ensure your application can handle region loss

**7. Document regions**: Keep clear documentation of what each region contains

Next steps
----------

- :doc:`08_migration`: Migrate regions between targets and providers
- :doc:`09_async`: Advanced async patterns for region operations
- :doc:`10_bedrock`: Production deployment with Bedrock
