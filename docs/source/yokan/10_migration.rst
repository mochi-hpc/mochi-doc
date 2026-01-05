Provider migration
==================

Yokan supports migrating database state from one provider to another. This
is useful for load balancing, service relocation, maintenance, and implementing
fault tolerance.

What is provider migration?
----------------------------

Provider migration transfers the complete state of a Yokan database from a
source provider to a destination provider. After migration:

- The destination provider contains all key/value pairs from the source
- The source can optionally be cleared (depending on migration mode)
- Both providers continue running (migration doesn't shut down providers)

Use cases for migration include:

- **Load balancing**: Move databases to less-loaded nodes
- **Maintenance**: Evacuate databases before node shutdown
- **Tiering**: Move data between storage tiers (memory → disk)
- **Replication**: Copy databases for redundancy
- **Disaster recovery**: Restore from backups

Migration API
-------------

The migration API is available in both C and C++.

C API
^^^^^

The C API is defined in ``yokan/database.h``:

.. code-block:: c

   yk_return_t yk_migrate_database(
       yk_database_handle_t source,      // Source database handle
       const char*          dest_addr,   // Destination address
       uint16_t             dest_provider_id,  // Destination provider ID
       const char*          migration_config,  // Migration options (JSON)
       bool                 remove_source      // Whether to clear source
   );

C++ API
^^^^^^^

The C++ API is defined in ``yokan/cxx/database.hpp``:

.. code-block:: cpp

   void yokan::Database::migrate(
       const std::string& dest_addr,
       uint16_t dest_provider_id,
       const std::string& migration_config = "{}",
       bool remove_source = false
   );

Basic migration example
------------------------

Here's a simple example of migrating a database:

.. literalinclude:: ../../../code/yokan/10_migration/migrate_basic.c
   :language: c

This example:

1. Connects to both source and destination providers
2. Populates the source database
3. Migrates all data to the destination
4. Verifies the migration succeeded
5. Source data is preserved (``remove_source = false``)

Migration with removal
----------------------

To move data (rather than copy), use ``remove_source = true``:

.. literalinclude:: ../../../code/yokan/10_migration/migrate_remove.c
   :language: c

After this migration, the source database will be empty.

Migration configuration
-----------------------

The ``migration_config`` parameter accepts a JSON object with options:

**Batch size**:

.. code-block:: json

   {
       "batch_size": 1000
   }

Controls how many key/value pairs are transferred per RPC. Larger batches
improve throughput but use more memory.

**Selection filter**:

.. code-block:: json

   {
       "selection": {
           "filter": "prefix:",
           "mode": "prefix"
       }
   }

Migrates only key/value pairs matching the filter. Modes include:

- ``prefix``: Keys starting with the filter string
- ``suffix``: Keys ending with the filter string
- ``lua``: Lua script filter (like in list operations)

**Example with filter**:

.. literalinclude:: ../../../code/yokan/10_migration/migrate_filter.c
   :language: c

Backend compatibility
---------------------

Migration works between any two backends that support the same data model:

**Key/value backends** (all backends):
- map ↔ map ✓
- map ↔ rocksdb ✓
- rocksdb ↔ lmdb ✓
- etc.

**Document backends** (sorted backends only):
- Cannot migrate documents to non-sorted backends
- Verify destination backend supports documents

.. warning::
   Migrating from a sorted backend (map, rocksdb, lmdb) to an unsorted
   backend (unordered_map, unordered_set) will preserve all data but lose
   ordering guarantees.

Migration with Bedrock
----------------------

When using Bedrock, providers can be migrated programmatically or via
runtime configuration APIs.

Programmatic migration
^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../code/yokan/10_migration/migrate_bedrock.c
   :language: c

Using Bedrock runtime API
^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bedrock ServiceHandle API also supports provider migration:

.. code-block:: cpp

   #include <bedrock/ServiceHandle.hpp>

   bedrock::ServiceHandle service = client.makeServiceHandle(addr, 0);

   service.migrateProvider(
       "yokan_provider_name",
       dest_addr,
       dest_provider_id,
       migration_config,
       remove_source
   );

Error handling
--------------

Migration can fail for various reasons:

.. literalinclude:: ../../../code/yokan/10_migration/migrate_error.c
   :language: c

Common errors:

- ``YOKAN_ERR_INVALID_ARGS``: Invalid destination address or provider ID
- ``YOKAN_ERR_NO_PERM``: Destination provider doesn't allow writes
- Network errors: Connection failures
- Backend errors: Insufficient space, write failures

Best practices
--------------

1. **Verify destination**: Ensure the destination provider exists and is
   accessible before migrating.

2. **Check capacity**: Verify the destination has sufficient storage space.

3. **Use appropriate batch sizes**:
   - Small batches (100-1000): Lower memory, slower
   - Large batches (10000+): Higher memory, faster

4. **Test migrations**: Test with a subset of data first using filters.

5. **Monitor progress**: For large databases, consider implementing progress
   tracking in your application.

6. **Handle failures gracefully**: Migration failures leave both providers
   in consistent states, but you may want to retry.

7. **Consider downtime**: While migration doesn't require shutting down
   providers, you may want to prevent writes during migration to ensure
   consistency.

Performance considerations
--------------------------

Migration performance depends on:

- **Database size**: Larger databases take longer
- **Network bandwidth**: Higher bandwidth improves throughput
- **Batch size**: Larger batches improve efficiency
- **Backend speed**: Faster backends (memory) migrate quicker than slower ones (disk)

**Typical throughput**:

- In-memory to in-memory: 100K-500K keys/sec
- Disk to disk: 10K-100K keys/sec
- Cross-network: Limited by bandwidth

Advanced: Incremental migration
--------------------------------

For very large databases, consider incremental migration:

.. literalinclude:: ../../../code/yokan/10_migration/migrate_incremental.c
   :language: c

This approach:

- Migrates data in chunks based on key prefixes
- Allows monitoring and cancellation
- Reduces memory pressure
- Enables progress reporting

Migration vs. Replication
-------------------------

**Migration** transfers data once:
- One-time operation
- Source can be cleared
- No ongoing synchronization

**Replication** (if supported by backend):
- Continuous synchronization
- Multiple replicas maintained
- Higher complexity

Yokan's migration API is designed for one-time transfers. For continuous
replication, consider application-level solutions or backend-specific features.

Limitations
-----------

1. **No automatic rollback**: If migration fails partway, manual cleanup
   may be needed.

2. **No live consistency**: Writes during migration may not be captured.

3. **Memory overhead**: Large batch sizes require proportional memory.

4. **Backend-specific**: Some backends may have limitations on migration.

Troubleshooting
---------------

**Migration hangs**:
- Check network connectivity
- Verify destination provider is responding
- Check for deadlocks or resource exhaustion

**Partial migration**:
- Review error logs
- Check destination storage capacity
- Verify permissions

**Performance issues**:
- Increase batch size
- Use faster network
- Consider backend performance characteristics

Next steps
----------

- :doc:`02_advanced_setup`: Configure high-performance deployments
- :doc:`07_backends`: Learn about different backend capabilities
- :doc:`../bedrock/07_runtime_config`: Use Bedrock for runtime migration
