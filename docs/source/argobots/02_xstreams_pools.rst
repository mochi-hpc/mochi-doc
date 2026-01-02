Execution Streams and Pools
============================

In this tutorial, you will learn how to create multiple execution streams for parallel
execution and understand how pools distribute work among them. This is essential for
achieving high performance in Mochi applications and understanding how Bedrock configures
Argobots for your services.

Prerequisites
-------------

- Completed Tutorial 01 (Introduction and Basic Execution)
- Understanding of basic parallelism concepts
- Multi-core system (recommended for observing parallel execution)

What You'll Learn
-----------------

By the end of this tutorial, you will understand:

- How to create multiple execution streams for parallelism
- The concept of pools and their role in work distribution
- Different pool types and access modes
- Fixed allocation vs. work-stealing strategies
- How execution streams interact with pools
- When to use each strategy in Mochi applications

Key Concepts
------------

**Execution Streams (xstreams)**
  Execution streams are OS-level threads that execute Argobots work units (ULTs and
  tasklets). Each execution stream runs independently and can execute work in parallel
  with other streams. Think of them as worker threads.

  **Best Practice**: Create one execution stream per CPU core for optimal performance.
  Creating more execution streams than cores can lead to contention and reduced performance.

**Pools**
  Pools are work queues that hold ULTs and tasklets waiting to be executed. Each execution
  stream has at least one pool that it pulls work from. Pools can be:

  - **Private**: Only one execution stream accesses the pool (lower overhead)
  - **Shared**: Multiple execution streams can access the pool (enables work-stealing)

**Pool Types**
  Argobots provides several predefined pool types:

  - ``ABT_POOL_FIFO``: First-In-First-Out queue (simple, low overhead)
  - ``ABT_POOL_FIFO_WAIT``: FIFO with blocking wait when empty
  - ``ABT_POOL_RANDWS``: Random work-stealing pool (best for load balancing)

**Pool Access Modes**
  Access modes control thread safety and determine which execution streams can access a pool:

  - ``ABT_POOL_ACCESS_PRIV``: Single Producer, Single Consumer (private to one xstream)
  - ``ABT_POOL_ACCESS_SPSC``: Single Producer, Single Consumer (optimized lock-free)
  - ``ABT_POOL_ACCESS_MPSC``: Multiple Producers, Single Consumer
  - ``ABT_POOL_ACCESS_SPMC``: Single Producer, Multiple Consumers
  - ``ABT_POOL_ACCESS_MPMC``: Multiple Producers, Multiple Consumers (full thread-safe)

**Work Distribution Strategies**

  1. **Fixed Allocation**: Each execution stream has its own private pool. ULTs are
     statically assigned to pools and never migrate. Simple but can lead to load imbalance.

  2. **Work-Stealing**: Execution streams share pools. When an execution stream's pool
     is empty, it can "steal" work from other pools. Better load balancing but higher
     overhead due to synchronization.

Fixed Allocation Example
-------------------------

In fixed allocation, each execution stream has its own private pool. ULTs are assigned
to specific pools and will only execute on that pool's execution stream.

.. literalinclude:: ../../../code/argobots/02_xstreams_pools/fixed_allocation.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Private Pools (lines 47-52)**
  Each execution stream automatically gets a default private pool when created with
  ``ABT_xstream_create(ABT_SCHED_NULL, &xstream)``. We retrieve these pools using
  ``ABT_xstream_get_main_pools()``.

**Static Assignment (lines 55-63)**
  ULTs are assigned to pools in round-robin fashion. Once assigned, a ULT will only
  execute on that pool's execution stream. This is simple and has low overhead since
  pools don't need synchronization.

**Advantages:**
  - Lower overhead (no lock contention)
  - Predictable execution (ULT always runs on same execution stream)
  - Better cache locality

**Disadvantages:**
  - Load imbalance: Some execution streams may finish early while others are still busy
  - No dynamic load balancing

Work-Stealing Example
----------------------

In work-stealing, execution streams can access multiple pools. When an execution stream
runs out of work in its own pool, it can steal work from other pools.

.. literalinclude:: ../../../code/argobots/02_xstreams_pools/work_stealing.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Creating Shared Pools (lines 49-54)**
  .. code-block:: c

     ABT_pool_create_basic(ABT_POOL_FIFO,           /* Pool type */
                           ABT_POOL_ACCESS_MPMC,    /* Access mode */
                           ABT_TRUE,                /* Automatic free */
                           &pools[i]);

  The ``ABT_POOL_ACCESS_MPMC`` access mode makes the pool thread-safe, allowing multiple
  execution streams to safely push and pop work.

**Scheduler with Multiple Pools (lines 57-71)**
  .. code-block:: c

     /* Pool priority order: own pool first, then others */
     for (j = 0; j < NUM_XSTREAMS; j++) {
         sched_pools[j] = pools[(i + j) % NUM_XSTREAMS];
     }
     ABT_sched_create_basic(ABT_SCHED_DEFAULT, NUM_XSTREAMS,
                            sched_pools, ABT_SCHED_CONFIG_NULL, &scheds[i]);

  Each scheduler gets access to all pools, ordered by priority. The scheduler first
  checks its primary pool, then tries others in round-robin order. This enables
  work-stealing.

**Varying Workloads (lines 24-33)**
  The example simulates varying work amounts to demonstrate work-stealing. ULTs with
  heavy work take longer, allowing execution streams with light work to steal from
  pools that still have pending ULTs.

**Advantages:**
  - Better load balancing (idle execution streams steal work)
  - More efficient use of resources
  - Handles dynamic and unpredictable workloads well

**Disadvantages:**
  - Higher overhead (synchronization costs)
  - Less predictable execution
  - Potential cache thrashing from migration

Building and Running the Examples
----------------------------------

Build both examples using CMake:

.. code-block:: bash

   cd code/argobots/02_xstreams_pools
   mkdir build && cd build
   cmake ..
   make

Run the fixed allocation example:

.. code-block:: bash

   ./fixed_allocation

Expected output (order may vary):

.. code-block:: text

   === Fixed Allocation Example ===
   Creating 4 execution streams with private pools

   ULT  0 executing on ES 0 (fixed allocation)
   ULT  4 executing on ES 0 (fixed allocation)
   ULT  8 executing on ES 0 (fixed allocation)
   ULT 12 executing on ES 0 (fixed allocation)
   ULT  1 executing on ES 1 (fixed allocation)
   ULT  5 executing on ES 1 (fixed allocation)
   ...
   All ULTs completed
   Note: Each ULT executed only on its assigned execution stream

Notice that ULT IDs are grouped by execution stream: 0,4,8,12 on ES 0; 1,5,9,13 on ES 1; etc.

Run the work-stealing example:

.. code-block:: bash

   ./work_stealing

Expected output (order will vary significantly):

.. code-block:: text

   === Work-Stealing Example ===
   Creating 4 execution streams with shared pools
   Creating 16 ULTs (some with heavy work)...

   ULT  1 executing on ES 1 (light work)
   ULT  2 executing on ES 2 (light work)
   ULT  0 executing on ES 0 (heavy work)
   ULT  3 executing on ES 3 (light work)
   ULT  5 executing on ES 2 (light work)
   ...
   All ULTs completed
   Note: ULTs may have executed on different execution streams
         due to work-stealing for better load balancing

Notice that ULTs are not strictly grouped by execution stream. Execution streams that
finish their light work may steal heavy work from others.

Understanding Pool Access Modes
--------------------------------

Choosing the right access mode is critical for performance:

**ABT_POOL_ACCESS_PRIV / SPSC**
  Use for private pools accessed by only one execution stream. Lowest overhead.

  .. code-block:: c

     /* Default pools created by ABT_xstream_create() are private */
     ABT_xstream_create(ABT_SCHED_NULL, &xstream);

**ABT_POOL_ACCESS_MPSC**
  Use when multiple execution streams create work but only one executes it.
  Common in producer-consumer patterns.

**ABT_POOL_ACCESS_MPMC**
  Use for work-stealing pools where multiple execution streams both produce and
  consume work. Highest overhead but most flexible.

  .. code-block:: c

     ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                           ABT_TRUE, &pool);

When to Use Each Strategy
--------------------------

**Use Fixed Allocation When:**
  - Workload is balanced and predictable
  - Each task belongs to a specific domain (e.g., processing separate data partitions)
  - Minimizing overhead is critical
  - Cache locality is important
  - Example: Margo RPC handlers on dedicated pools

**Use Work-Stealing When:**
  - Workload is unbalanced or unpredictable
  - Task execution times vary significantly
  - Maximizing throughput is more important than latency
  - You have more tasks than execution streams
  - Example: Task-parallel algorithms, recursive divide-and-conquer

Mochi/Bedrock Connection
-------------------------

Understanding execution streams and pools is crucial for configuring Mochi services
through Bedrock. Bedrock configurations allow you to:

- Create custom pools for different types of work
- Assign Margo RPC handlers to specific pools
- Configure work-stealing for load balancing
- Set pool access modes for optimal performance

Example Bedrock pool configuration:

.. code-block:: json

   {
       "argobots": {
           "pools": [
               {
                   "name": "rpc_pool",
                   "kind": "fifo",
                   "access": "mpmc"
               },
               {
                   "name": "io_pool",
                   "kind": "fifo_wait",
                   "access": "mpmc"
               }
           ],
           "xstreams": [
               {
                   "name": "rpc_xstream",
                   "scheduler": {
                       "type": "basic_wait",
                       "pools": ["rpc_pool"]
                   }
               },
               {
                   "name": "io_xstream",
                   "scheduler": {
                       "type": "basic_wait",
                       "pools": ["io_pool", "rpc_pool"]
                   }
               }
           ]
       }
   }

This configuration creates dedicated pools and execution streams for RPC and I/O
operations, with the I/O execution stream able to steal work from the RPC pool.

Common Pitfalls
---------------

**Too Many Execution Streams**
  Creating more execution streams than CPU cores often reduces performance due to
  context switching overhead and cache contention.

  .. code-block:: c

     /* WRONG: 100 execution streams on a 4-core machine */
     for (i = 0; i < 100; i++) {
         ABT_xstream_create(ABT_SCHED_NULL, &xstreams[i]);
     }

  **Best Practice**: Use ``sched_getaffinity()`` or environment variables to determine
  the number of available cores.

**Forgetting to Join Execution Streams**
  Always join and free secondary execution streams before finalizing Argobots:

  .. code-block:: c

     /* Must join and free all secondary execution streams */
     for (i = 1; i < num_xstreams; i++) {
         ABT_xstream_join(xstreams[i]);
         ABT_xstream_free(&xstreams[i]);
     }

**Wrong Pool Access Mode**
  Using ``ABT_POOL_ACCESS_PRIV`` for a shared pool causes data races:

  .. code-block:: c

     /* WRONG: Private access mode for shared pool */
     ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_PRIV,
                           ABT_TRUE, &shared_pool);
     /* Multiple xstreams accessing this pool = undefined behavior */

  **Fix**: Use ``ABT_POOL_ACCESS_MPMC`` for work-stealing pools.

**Not Freeing Pools Created with ABT_pool_create_basic**
  If you create pools manually and set ``automatic`` to ``ABT_FALSE``, you must free them:

  .. code-block:: c

     ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC,
                           ABT_FALSE, &pool);  /* Manual free */
     /* ... use pool ... */
     ABT_pool_free(&pool);  /* Must free manually */

API Reference
-------------

This tutorial covered the following Argobots functions:

**Execution Stream Functions**
  - ``int ABT_xstream_create(ABT_sched sched, ABT_xstream *newxstream)``

    Create a new execution stream with the specified scheduler. Pass ``ABT_SCHED_NULL``
    to use the default scheduler.

  - ``int ABT_xstream_create_basic(ABT_sched_predef predef, int num_pools, ABT_pool *pools, ABT_sched_config config, ABT_xstream *newxstream)``

    Create an execution stream with a predefined scheduler and specific pools.

  - ``int ABT_xstream_join(ABT_xstream xstream)``

    Wait for an execution stream to terminate. Must be called before freeing.

  - ``int ABT_xstream_free(ABT_xstream *xstream)``

    Free an execution stream. Must call ``ABT_xstream_join()`` first.

  - ``int ABT_xstream_self_rank(int *rank)``

    Get the rank of the current execution stream (0 for primary, 1+ for secondary).

  - ``int ABT_xstream_set_main_sched(ABT_xstream xstream, ABT_sched sched)``

    Set the main scheduler for an execution stream.

**Pool Functions**
  - ``int ABT_pool_create_basic(ABT_pool_kind kind, ABT_pool_access access, ABT_bool automatic, ABT_pool *newpool)``

    Create a pool with predefined kind and access mode.

    **Parameters**:
      - ``kind``: Pool type (FIFO, FIFO_WAIT, RANDWS)
      - ``access``: Access mode (PRIV, SPSC, MPSC, SPMC, MPMC)
      - ``automatic``: If ``ABT_TRUE``, pool is automatically freed
      - ``newpool``: Output handle for the created pool

  - ``int ABT_pool_free(ABT_pool *pool)``

    Free a pool (only if created with ``automatic = ABT_FALSE``).

**Scheduler Functions**
  - ``int ABT_sched_create_basic(ABT_sched_predef predef, int num_pools, ABT_pool *pools, ABT_sched_config config, ABT_sched *newsched)``

    Create a scheduler with predefined type and specific pools.

Next Steps
----------

Now that you understand execution streams and pools, you can move on to:

- **Tutorial 03: ULTs vs Tasklets** - Learn about the two types of work units and when
  to use each.

- **Tutorial 04: Schedulers and Work Distribution** - Deep dive into predefined
  schedulers and scheduling policies.

For Mochi developers, understanding pools and execution streams is essential for:

- Configuring Bedrock to optimize service performance
- Creating dedicated pools for different types of work (RPC, I/O, computation)
- Tuning work-stealing behavior for load balancing
