Advanced setup and configuration
=================================

This tutorial covers advanced configuration options for Yokan providers,
including performance tuning, pool configuration, backend selection, and
security considerations.

Choosing the right backend
---------------------------

Yokan supports multiple storage backends, each with different characteristics:

**In-Memory Backends** (no persistence):
- **map**: Ordered key/value store using C++ std::map (supports range queries)
- **unordered_map**: Hash-based key/value store (faster lookups, no ordering)
- **set**: Ordered set (keys only, no values)
- **unordered_set**: Hash-based set (keys only)

**Persistent Backends** (require Spack variants):
- **berkeleydb**: High-performance embedded database
- **leveldb**: Fast key/value store optimized for writes
- **rocksdb**: High-performance, LSM-tree based (best for write-heavy workloads)
- **lmdb**: Memory-mapped database (excellent read performance)
- **gdbm**: GNU database manager (simple, reliable)
- **tkrzw**: Modern key/value store with multiple implementations
- **unqlite**: Embedded NoSQL database with document store support

**Selection guidelines**:

- Use **map** for development, testing, or temporary data
- Use **rocksdb** for write-heavy workloads or large datasets
- Use **lmdb** for read-heavy workloads
- Use **leveldb** for good balance of read/write performance
- Use **tkrzw** for advanced features and flexibility

Pool and execution stream configuration
----------------------------------------

For high-performance deployments, configure dedicated Argobots pools
for Yokan operations:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/dedicated-pool.json
   :language: json

This configuration:
- Creates a dedicated RPC pool with 4 execution streams
- Binds execution streams to specific CPU cores
- Isolates Yokan operations from other services

Performance tuning parameters
------------------------------

Backend-specific configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**RocksDB configuration**:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/rocksdb-config.json
   :language: json

Key parameters:
- ``write_buffer_size``: Memory used for writes before flushing
- ``max_write_buffer_number``: Number of write buffers
- ``compression``: Compression algorithm (none, snappy, zlib, lz4)
- ``block_cache_size``: Cache size for read operations

**LMDB configuration**:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/lmdb-config.json
   :language: json

Key parameters:
- ``map_size``: Maximum database size (must be set appropriately)
- ``no_sync``: Disable sync for better write performance (less safe)
- ``no_metasync``: Disable metadata sync

Bulk transfer optimization
^^^^^^^^^^^^^^^^^^^^^^^^^^^

For large key/value pairs, configure bulk transfer settings:

.. code-block:: json

   {
       "config": {
           "database": {
               "type": "rocksdb",
               "config": {
                   "bulk_cache_size": 1048576,
                   "use_bulk_cache": true
               }
           }
       }
   }

This enables caching of bulk transfer handles for improved performance.

Disable RDMA for small data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For small key/value pairs (< 4KB), using ``YOKAN_MODE_NO_RDMA`` can
improve performance by avoiding RDMA overhead:

.. code-block:: c

   // Client code
   yk_put(dbh, "key", 3, "value", 5, YOKAN_MODE_NO_RDMA);

Multi-database configuration
-----------------------------

A single provider can manage multiple databases:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/multi-database.json
   :language: json

This is useful for:
- Separating different data types
- Using different backends for different workloads
- Implementing data tiers (hot/warm/cold)

Dependencies and integration
-----------------------------

Yokan providers can depend on other Mochi services:

Integration with ABT-IO
^^^^^^^^^^^^^^^^^^^^^^^

For backends that use file I/O, integrate with ABT-IO for better async performance:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/with-abtio.json
   :language: json

Integration with Flock
^^^^^^^^^^^^^^^^^^^^^^

For distributed deployments, integrate with Flock for service discovery:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/with-flock.json
   :language: json

This allows Yokan providers to discover each other in a group for
coordinated operations or replication.

Security considerations
-----------------------

Authentication and authorization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

While Yokan itself doesn't provide built-in authentication, you can
integrate it with Mochi's authentication framework:

.. code-block:: json

   {
       "margo": {
           "enable_abt_profiling": false,
           "use_progress_thread": true,
           "enable_diagnostics": false
       }
   }

Data encryption
^^^^^^^^^^^^^^^

For sensitive data:
1. Use encrypted storage backends (e.g., encrypted filesystem)
2. Implement application-level encryption before storing
3. Use secure transport protocols (see Margo documentation)

Best practices
--------------

1. **Start with defaults**: Begin with the map backend and default settings,
   then optimize based on profiling.

2. **Match backend to workload**:
   - Write-heavy: RocksDB
   - Read-heavy: LMDB
   - Balanced: LevelDB or TKRZW

3. **Use appropriate pools**: For high-throughput scenarios, use dedicated
   pools to avoid contention.

4. **Monitor performance**: Use Margo's profiling capabilities to identify
   bottlenecks.

5. **Size buffers appropriately**: Match buffer sizes to your typical
   key/value sizes to avoid waste or frequent reallocations.

6. **Consider persistence requirements**: If data doesn't need to survive
   restarts, use in-memory backends for better performance.

7. **Test backend configurations**: Different backends and configurations
   can have vastly different performance characteristics for your workload.

Example: High-performance configuration
----------------------------------------

Here's a complete example for a high-performance Yokan deployment:

.. literalinclude:: ../../../code/yokan/02_advanced_setup/high-performance.json
   :language: json

This configuration:
- Uses RocksDB for persistence
- Dedicated pools for Yokan operations
- Optimized for write-heavy workloads
- Integrates with ABT-IO for async I/O
- Uses compression to save space

Troubleshooting
---------------

**Database won't start**:
- Check backend is installed (Spack variants)
- Verify file permissions for persistent backends
- Check map_size for LMDB (must be large enough)

**Poor performance**:
- Profile with Margo diagnostics
- Check if operations are using RDMA unnecessarily
- Verify pool configuration
- Consider backend-specific tuning

**Out of memory**:
- Reduce write buffer sizes
- Lower cache sizes
- Use compression
- Consider switching backends

Next steps
----------

- :doc:`03_basics`: Learn basic database operations
- :doc:`07_backends`: Detailed backend documentation
- :doc:`05_modes`: Learn about operation modes for performance tuning
