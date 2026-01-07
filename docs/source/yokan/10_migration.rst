Database migration
==================

Yokan supports migrating a database from one provider to another using REMI
(REsource MIgration component). This is useful for relocating databases,
balancing load, or moving data between storage tiers.

.. important::
   Database migration requires:

   - Yokan compiled with the ``+remi`` variant
   - A REMI receiver provider on the destination
   - A REMI sender provider on the source
   - The destination provider initialized with an empty configuration: ``"{}"``

What is database migration?
----------------------------

Database migration physically transfers the database files from a source
provider to a destination provider. After migration:

- The destination provider takes ownership of the database
- The source provider's database becomes invalid
- All key/value pairs and collections are transferred
- The database can continue being accessed at the new location

Migration uses REMI to transfer the database files efficiently over the network.

Setting up for migration
-------------------------

**Compiling Yokan with REMI support**:

.. code-block:: console

   spack install mochi-yokan +remi

**Provider setup for migration**:

The source and destination providers must be configured with REMI support:

.. code-block:: c

   #include <remi/remi-server.h>
   #include <remi/remi-client.h>

   // Register a REMI provider (needed on destination)
   remi_provider_t remi_provider;
   remi_provider_register(
       mid, ABT_IO_INSTANCE_NULL,
       provider_id, ABT_POOL_NULL, &remi_provider);

   // Create a REMI client (needed on source)
   remi_client_t remi_client;
   remi_client_init(mid, ABT_IO_INSTANCE_NULL, &remi_client);

   // Source provider: needs REMI client
   struct yk_provider_args args1 = YOKAN_PROVIDER_ARGS_INIT;
   args1.remi.client = remi_client;
   args1.remi.provider = REMI_PROVIDER_NULL;
   yk_provider_register(mid, 1, config, &args1, &source_provider);

   // Destination provider: needs REMI provider and EMPTY config
   struct yk_provider_args args2 = YOKAN_PROVIDER_ARGS_INIT;
   args2.remi.client = REMI_CLIENT_NULL;
   args2.remi.provider = remi_provider;
   yk_provider_register(mid, 2, "{}", &args2, &dest_provider);
   //                            ^^^^
   //                            MUST be empty!

The destination provider **must** be initialized with an empty configuration
``"{}"`` because it will receive its database through migration.

Migration API
-------------

The migration function is called from the source provider:

.. code-block:: c

   yk_return_t yk_provider_migrate_database(
       yk_provider_t provider,                        // Source provider
       const char* dest_addr,                         // Destination address
       uint16_t dest_provider_id,                     // Destination provider ID
       const struct yk_migration_options* options     // Migration options
   );

**Migration options structure**:

.. code-block:: c

   struct yk_migration_options {
       const char* new_root;      // New path for database on destination
       const char* extra_config;  // Extra JSON config for destination
       size_t      xfer_size;     // Transfer block size (0 = default)
   };

Migration example
-----------------

Here's a complete example showing database migration:

.. literalinclude:: ../../../code/yokan/10_migration/migrate_example.c
   :language: c

This example:

1. Sets up both source and destination providers with REMI support
2. Creates a database on the source provider and populates it
3. Migrates the database to the destination provider
4. Verifies the source provider's database is now invalid
5. Verifies the destination provider now has the migrated data

Migration behavior
------------------

**After migration**:

- The source provider returns ``YOKAN_ERR_INVALID_DATABASE`` for operations
- The destination provider can now access the migrated database
- All key/value pairs are transferred
- All collections and documents are transferred
- The database files are physically moved or copied to the new location

**Migration options**:

- ``new_root``: Specifies the filesystem path where the database will be stored
  on the destination. This is important when the destination needs the database
  in a specific location (e.g., on a particular disk or mount point).

- ``extra_config``: Additional JSON configuration to apply at the destination.
  For example, you might change backend parameters while migrating.

- ``xfer_size``: Controls the transfer block size for REMI. Setting this to 0
  uses REMI's default. Larger sizes may improve throughput for large databases.

Backend compatibility
---------------------

Migration support depends on the backend's ability to have its files
transferred. In-memory backends without persistence can be migrated but they
will first dump their data into a temporary file, and send that file.
Some backends may return ``YOKAN_ERR_OP_UNSUPPORTED`` when migration
is attempted if migration is not suported..

Using migration with Bedrock
-----------------------------

When using Bedrock to manage Yokan providers, the setup is similar but
Bedrock handles provider registration. The key requirements remain:

1. Compile Yokan with ``+remi``
2. Register a "remi_sender" and/or a "remi_receiver" in your Bedrock configuration,
   depending on what you intend to support
3. Ensure the destination Yokan provider has an empty database configuration
4. Pass REMI sender/receiver as dependencies to the respective Yokan providers

Example Bedrock configuration:

.. code-block:: json

   {
       "libraries": [
           "libyokan-bedrock-module.so",
           "libremi-bedrock-module.so"
       ],
       "providers": [
           {
               "name": "sender",
               "type": "remi_sender",
               "provider_id": 1
           },
           {
               "name": "receiver",
               "type": "remi_receiver",
               "provider_id": 2
           },
           {
               "name": "yokan_source",
               "type": "yokan",
               "provider_id": 3,
               "config": {
                   "database": {
                       "type": "map"
                   }
               },
               "dependencies": {
                   "remi_sender": "sender"
               }
           },
           {
               "name": "yokan_dest",
               "type": "yokan",
               "provider_id": 4,
               "config": {},
               "dependencies": {
                   "remi_receiver": "receiver"
               }
           }
       ]
   }

Error handling
--------------

Common errors during migration:

- ``YOKAN_ERR_OP_UNSUPPORTED``: Backend doesn't support migration
- ``YOKAN_ERR_INVALID_DATABASE``: Source database is invalid or already migrated
- REMI errors: File transfer failures, permission issues
- Network errors: Connection failures to destination

After a failed migration attempt, the source database typically remains
valid and can be retried.

Limitations
-----------

- Migration is an all-or-nothing operation (no partial migration)
- No built-in support for incremental migration
- Source database becomes invalid after migration
- Not all backends support migration
