Transfer managers
=================

Transfer managers control how data is transferred between clients and Warabi providers.
They allow you to optimize for different scenarios: single large transfers, pipelined transfers, or concurrent transfers.

What are transfer managers?
----------------------------

A transfer manager is a strategy for moving data between client and server. Different
strategies offer different trade-offs between throughput, latency, and resource usage.

Warabi provides two built-in transfer managers:

- **Default**: Simple single-transfer strategy, RDMA goes directly to destination
- **Pipeline**: RDMA goes to pre-allocated buffers that are copied to destination in a pipeline manner

It is difficult to evaluate in which situation one would be better than the other, so
we encourage users give both a try.

Configuration
-------------

Transfer managers are configured at the provider level, using the "transfer_manager" field.

Example in a bedrock configuration:

.. literalinclude:: ../../../code/warabi/06_transfer_managers/bedrock-config.json
   :language: json

Default transfer manager
-------------------------

The default transfer manager performs operations in a single transfer to/from the target region.

**Configuration**: No configuration needed (used when transfer_manager is not specified)

.. code-block:: json

   {
       "config": {
           "target": {"type": "memory", "config": {}}
           // No transfer_manager specified = use default
       }
   }

**Behavior**:

- Single bulk transfer per operation
- Straightforward memory management
- Good for most use cases

Pipeline transfer manager
-------------------------

The pipeline transfer manager splits large transfers into chunks and pipelines
them with actual I/O. It requires the data be transferred to intermediate buffers
first. These buffers are pre-registered.

**Configuration options**:

.. code-block:: json

   {
       "transfer_manager": {
           "type": "pipeline",
           "config": {
                "num_pools": 4,
                "num_buffers_per_pool": 8,
                "first_buffer_size": 1048576,
                "buffer_size_multiplier": 2
           }
       }
   }

- :code:`num_pools`: Number of buffer pools (set of pre-allocated buffers with the same size)
- :code:`num_buffers_per_pool`: Number of buffers per pool
- :code:`first_buffer_size`: Size (in bytes) of the smallest buffers
- :code:`buffer_size_multiplier`: by how much to multiply the size from buffer pool N to buffer pool N+1
