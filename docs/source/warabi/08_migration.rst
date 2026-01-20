Target migration
================

Warabi supports migrating a target from one provider to another using REMI
(REsource MIgration component). This transfers all regions from the source
provider to the destination provider, useful for relocating storage, load
balancing, or moving data between backends.

.. important::
   Target migration requires:

   - Warabi compiled with REMI support (``+remi`` variant)
   - A REMI receiver (provider) on the destination
   - A REMI sender (client) on the source
   - The destination provider initialized with an empty configuration: ``"{}"``

What is target migration?
--------------------------

Target migration physically transfers an entire target (and all its regions)
from a source provider to a destination provider. After migration:

- The destination provider contains all regions from the source
- The source provider's target becomes invalid (if ``remove_source`` is true)
- All region IDs remain valid and can be accessed from the destination
- Migration uses REMI to transfer the target data efficiently

Migration uses REMI to transfer the underlying storage files efficiently
over the network.

Setting up for migration
-------------------------

**Compiling Warabi with REMI support**:

.. code-block:: console

   spack install mochi-warabi +remi

**Provider setup for migration**:

The source and destination providers must be configured with REMI support:

.. code-block:: cpp

   #include <remi/remi-server.h>
   #include <remi/remi-client.h>
   #include <warabi/Provider.hpp>

   // Initialize REMI
   remi_client_t remi_client;
   remi_client_init(mid, ABT_IO_INSTANCE_NULL, &remi_client);

   remi_provider_t remi_provider;
   remi_provider_register(mid, ABT_IO_INSTANCE_NULL,
                          provider_id, ABT_POOL_NULL, &remi_provider);

   // Source provider: needs REMI client, has a configured target
   std::string source_config = R"({
       "target": {
           "type": "memory",
           "config": {}
       }
   })";
   warabi::Provider provider1(engine, 1, source_config,
                              pool, remi_client, REMI_PROVIDER_NULL);

   // Destination provider: needs REMI provider, has EMPTY config
   warabi::Provider provider2(engine, 2, "{}", pool, REMI_CLIENT_NULL, remi_provider);
   //                                    ^^^^
   //                                    MUST be empty!

The destination provider **must** be initialized with an empty configuration
``"{}"`` (or at least a configuration without the "target" field) because it will
receive its target through migration.

Migration API
-------------

The migration function is called from the source provider:

.. code-block:: cpp

   void Provider::migrateTarget(
       const std::string& address,     // Destination address
       uint16_t provider_id,           // Destination provider ID
       const std::string& options      // Migration options (JSON)
   );

**Migration options** (JSON string):

.. code-block:: cpp

   std::string options = R"({
       "new_root": "/path/on/destination",
       "transfer_size": 1024,
       "merge_config": {},
       "remove_source": true
   })";

- ``new_root`` (string): Path where the target will be stored on the destination.
  Defaults to the same path as the source.

- ``transfer_size`` (int): Size of individual transfers used by REMI for the
  migration. Defaults to transferring the entire target in one operation.
  Smaller sizes reduce memory overhead but may be slower.

- ``merge_config`` (object): Additional configuration to merge with the target's
  configuration at the destination. Defaults to empty object.

- ``remove_source`` (bool): Whether to remove the target from the source provider
  after migration. Defaults to ``true``.

Migration example
-----------------

Here's a complete example showing target migration:

.. literalinclude:: ../../../code/warabi/08_migration/migration_example.cpp
   :language: cpp

This example:

1. Sets up both source and destination providers with REMI support
2. Creates regions in the source provider
3. Migrates the entire target to the destination provider
4. Verifies all regions are accessible from the destination
5. Confirms the source provider is now invalid

Migration behavior
------------------

**After migration**:

- The destination provider contains all regions that were in the source
- If ``remove_source`` is true (default), the source provider's target is removed
  and any operations on it will fail
- All region IDs remain valid and work with the destination provider
- The target files are physically moved or copied to the new location

**Migration options explained**:

- **new_root**: Specifies where the target will be stored on the destination.
  This is important when the destination needs the target in a specific location
  (e.g., on a particular disk or mount point).

  .. code-block:: cpp

     // Source has target at /mnt/fast-storage/warabi
     // Migrate to slow storage at /mnt/archive/warabi
     auto options = R"({
         "new_root": "/mnt/archive/warabi"
     })";

- **transfer_size**: Controls how much data is transferred in each REMI operation.
  Setting this to a smaller value reduces memory overhead but may increase
  latency. Setting it to 0 or omitting it transfers everything in one operation.

  .. code-block:: cpp

     // For large targets, use smaller chunks
     auto options = R"({
         "transfer_size": 1048576
     })";  // 1 MB chunks

- **merge_config**: Allows changing target configuration during migration.
  For example, you might migrate from a memory backend to a persistent backend.

  .. code-block:: cpp

     auto options = R"({
         "new_root": "/mnt/pmem/warabi",
         "merge_config": {
             "target": {
                 "type": "pmem",
                 "config": {
                     "path": "/mnt/pmem/warabi.pmem",
                     "create_if_missing_with_size": 10737418240
                 }
             }
         }
     })";

- **remove_source**: If ``false``, the source target is kept after migration,
  resulting in two copies. This is useful for creating backups.

  .. code-block:: cpp

     // Create a backup without removing source
     auto options = R"({
         "new_root": "/backup/warabi",
         "remove_source": false
     })";

Backend compatibility
---------------------

Migration works for backends that store data in files or pools that can be
transferred via REMI, i.e. the "pmem" and "abtio" backends. The "memory"
backend does not support migration.

Using migration with Bedrock
-----------------------------

When using Bedrock to manage Warabi providers, the setup is similar:

.. literalinclude:: ../../../code/warabi/08_migration/bedrock-config.json
   :language: json
