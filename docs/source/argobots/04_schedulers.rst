Schedulers and Work Distribution
==================================

In this tutorial, you will learn about Argobots schedulers and how they control the
distribution and execution of work units. Schedulers are the key to achieving optimal
performance through proper work distribution and load balancing.

Prerequisites
-------------

- Completed Tutorials 01-03
- Understanding of execution streams and pools
- Familiarity with work-stealing concepts from Tutorial 02

What You'll Learn
-----------------

By the end of this tutorial, you will understand:

- The role of schedulers in Argobots
- Different predefined scheduler types (BASIC, RANDWS, PRIO)
- How schedulers interact with pools
- Scheduler configuration options
- When to use each scheduler type
- Work-stealing for divide-and-conquer algorithms

Key Concepts
------------

**Schedulers**
  A scheduler is responsible for pulling work units from pools and executing them
  on an execution stream. Every execution stream has exactly one main scheduler.

  The scheduler's main loop:
  1. Check pools for available work units
  2. Select a work unit to execute
  3. Execute the work unit
  4. Repeat until no more work or shutdown signal

**Scheduler Types**

  - ``ABT_SCHED_DEFAULT``: Default scheduler (usually same as BASIC)
  - ``ABT_SCHED_BASIC``: Simple FIFO scheduler with blocking wait
  - ``ABT_SCHED_BASIC_WAIT``: BASIC with efficient waiting
  - ``ABT_SCHED_PRIO``: Priority scheduler (checks pools in order)
  - ``ABT_SCHED_RANDWS``: Random work-stealing scheduler

**Scheduler-Pool Relationship**
  - A scheduler can access multiple pools
  - Pools are checked in order (first pool = highest priority)
  - When the first pool is empty, scheduler checks the second pool, etc.
  - This enables work-stealing: schedulers steal from other pools

BASIC Scheduler
---------------

The BASIC scheduler is simple and efficient for single-pool scenarios:

.. literalinclude:: ../../../code/argobots/04_schedulers/basic_scheduler.c
   :language: c
   :linenos:

The BASIC scheduler:
- Pulls work units from its pool in FIFO order
- Blocks when the pool is empty (efficient waiting)
- Simple, low overhead
- Best for dedicated pools with steady work arrival

Priority Scheduler
------------------

The PRIO scheduler checks pools in priority order:

.. literalinclude:: ../../../code/argobots/04_schedulers/priority_scheduler.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Pool Priority (lines 38-39)**
  .. code-block:: c

     pools[0] = high_pool;  /* Checked first */
     pools[1] = low_pool;   /* Checked second */

  The PRIO scheduler always drains higher-priority pools before moving to lower-priority
  ones. This is useful for ensuring critical work executes first.

**Use Cases**:
  - RPC handlers (high priority) vs background tasks (low priority)
  - Latency-critical operations vs throughput-oriented operations
  - Interactive tasks vs batch processing

Work-Stealing Scheduler
------------------------

The RANDWS (random work-stealing) scheduler enables dynamic load balancing:

.. literalinclude:: ../../../code/argobots/04_schedulers/work_stealing.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Multi-Pool Access (lines 55-57)**
  Each scheduler gets access to all pools, ordered so its own pool is first.
  When a scheduler's pool is empty, it randomly steals from other pools.

**Dynamic Load Balancing**
  Work-stealing automatically balances load:
  - Execution streams with light work finish early
  - They steal heavy work from busy execution streams
  - Better utilization of all cores

**Overhead**
  Work-stealing has higher overhead than fixed allocation due to:
  - Synchronization on shared pools (MPMC access)
  - Random selection overhead
  - Cache effects from work migration

Fibonacci Example
-----------------

Recursive divide-and-conquer algorithms benefit greatly from work-stealing:

.. literalinclude:: ../../../code/argobots/04_schedulers/fibonacci.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Recursive Parallelism (lines 30-45)**
  Each fibonacci call spawns a child ULT for one branch and directly computes the other.
  This creates a tree of ULTs that work-stealing distributes across execution streams.

**Dynamic Work Distribution**
  Work-stealing is ideal for recursive algorithms because:
  - Work is created dynamically (can't predict load upfront)
  - Some branches complete faster than others
  - Idle execution streams can steal pending work

**Performance**
  With work-stealing, this fibonacci computation utilizes all cores effectively.
  Without it, work would be statically assigned and load imbalance would waste cores.

Building and Running Examples
------------------------------

Build all examples:

.. code-block:: bash

   cd code/argobots/04_schedulers
   mkdir build && cd build
   cmake ..
   make

Run each example:

.. code-block:: bash

   ./basic_scheduler
   ./priority_scheduler
   ./work_stealing
   ./fibonacci

Scheduler Configuration
-----------------------

Schedulers can be configured with additional parameters:

**Event Frequency**
  .. code-block:: c

     ABT_sched_config config;
     ABT_sched_config_create(&config,
         ABT_sched_config_var_end);
     ABT_sched_create_basic(ABT_SCHED_BASIC, 1, &pool, config, &sched);
     ABT_sched_config_free(&config);

**Automatic Free**
  By default, schedulers created with ``ABT_sched_create_basic()`` are automatically
  freed when their execution stream is freed. You can control this behavior.

Choosing a Scheduler
---------------------

**Use BASIC when:**
  - Single pool per execution stream
  - Work arrives at steady rate
  - Minimal overhead is important
  - No load balancing needed

**Use PRIO when:**
  - You have distinct work priority levels
  - Critical work must execute before background work
  - Multiple pool types (RPC, I/O, computation)
  - Willing to accept some overhead for prioritization

**Use RANDWS when:**
  - Workload is unpredictable or bursty
  - Recursive or divide-and-conquer algorithms
  - Maximum throughput is more important than minimal latency
  - You have multiple execution streams
  - Load balancing is critical

Mochi/Bedrock Integration
--------------------------

Bedrock configurations expose scheduler choices:

.. code-block:: json

   {
       "argobots": {
           "pools": [
               {"name": "rpc_pool", "kind": "fifo_wait", "access": "mpmc"},
               {"name": "io_pool", "kind": "fifo_wait", "access": "mpmc"}
           ],
           "xstreams": [
               {
                   "name": "rpc_stream",
                   "scheduler": {
                       "type": "basic_wait",
                       "pools": ["rpc_pool"]
                   }
               },
               {
                   "name": "io_stream",
                   "scheduler": {
                       "type": "basic_wait",
                       "pools": ["io_pool", "rpc_pool"]
                   }
               }
           ]
       }
   }

This configuration:
- Creates dedicated streams for RPC and I/O
- I/O stream can steal from RPC pool when idle
- BASIC_WAIT scheduler for efficient waiting

Common Scheduler Types in Bedrock:
- ``basic_wait``: Most common for Margo services
- ``prio``: For multi-priority workloads
- ``randws``: For computational workloads

Common Pitfalls
---------------

**Wrong Pool Access Mode for Work-Stealing**
  .. code-block:: c

     /* WRONG: PRIV access with multiple schedulers */
     ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_PRIV,
                           ABT_TRUE, &pool);
     /* Multiple schedulers accessing this = race conditions */

  For work-stealing, pools must use ``ABT_POOL_ACCESS_MPMC``.

**Too Many Pools per Scheduler**
  .. code-block:: c

     /* WRONG: Scheduler with too many pools */
     ABT_pool pools[100];
     ABT_sched_create_basic(ABT_SCHED_PRIO, 100, pools, ...);

  Each pool adds overhead. Usually 2-4 pools per scheduler is sufficient.

**Forgetting Pool Order Matters**
  .. code-block:: c

     /* Pool order is priority order */
     pools[0] = low_priority_pool;   /* Will be checked first! */
     pools[1] = high_priority_pool;  /* Checked second */

  The first pool has highest priority. Order matters for PRIO schedulers.

API Reference
-------------

**Scheduler Creation**
  - ``int ABT_sched_create_basic(ABT_sched_predef predef, int num_pools, ABT_pool *pools, ABT_sched_config config, ABT_sched *newsched)``

    Create a predefined scheduler.

    **Parameters**:
      - ``predef``: Scheduler type (DEFAULT, BASIC, BASIC_WAIT, PRIO, RANDWS)
      - ``num_pools``: Number of pools this scheduler accesses
      - ``pools``: Array of pool handles
      - ``config``: Configuration (use ABT_SCHED_CONFIG_NULL for defaults)
      - ``newsched``: Output scheduler handle

**Scheduler Management**
  - ``int ABT_xstream_set_main_sched(ABT_xstream xstream, ABT_sched sched)``

    Set the main scheduler for an execution stream.

  - ``int ABT_sched_finish(ABT_sched sched)``

    Request scheduler to finish (stop scheduling new work).

  - ``int ABT_sched_free(ABT_sched *sched)``

    Free a scheduler (only if not automatically freed).

**Configuration**
  - ``int ABT_sched_config_create(ABT_sched_config *config, ...)``

    Create scheduler configuration.

  - ``int ABT_sched_config_free(ABT_sched_config *config)``

    Free scheduler configuration.

Next Steps
----------

Now that you understand schedulers, you can move on to synchronization primitives:

- **Tutorial 05: Barriers** - Learn how to synchronize multiple work units at specific
  points in execution.

- **Tutorial 06: Futures** - Understand fine-grained dependency management for better
  parallelism.

For Mochi developers, understanding schedulers is crucial for:
- Bedrock configuration optimization
- Choosing the right scheduler for your workload
- Balancing latency vs throughput
- Configuring work-stealing for computational services
