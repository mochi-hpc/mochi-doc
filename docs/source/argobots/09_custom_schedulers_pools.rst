Custom Schedulers and Pools
============================

Argobots provides a powerful extension mechanism that allows you to implement custom
pool types and custom schedulers tailored to your application's specific needs.

.. note::

   Implementing custom pools and schedulers is an advanced topic that requires deep
   understanding of Argobots internals and the scheduling API. This is typically only
   needed for very specialized use cases.

Why Custom Pools and Schedulers?
---------------------------------

While Argobots comes with several predefined pool types (FIFO, FIFO_WAIT, RANDWS) and
schedulers (BASIC, RANDWS, PRIO), some applications may benefit from specialized
implementations:

**Custom Pools**:
  - Priority-based scheduling
  - LIFO (stack) pools for depth-first algorithms
  - Application-specific data structures
  - Lock-free implementations for specific access patterns
  - Locality-aware scheduling

**Custom Schedulers**:
  - Application-specific scheduling policies
  - Energy-aware scheduling
  - QoS-based scheduling
  - Integration with external scheduling systems
  - Multi-level scheduling hierarchies

Margo Examples
--------------

Rather than providing detailed implementation examples here, we recommend examining
the production-quality implementations in the Margo library, which is part of the
Mochi ecosystem:

**Margo Custom Pools**:

1. **Priority Pool ("prio_wait")**

   A priority-based pool where work units end up in either of two bins: a high priority
   one, and a low priority one. The user doesn't get to decide which bin is used.
   Newly-created ULTs fall into the high priority bin, while ULTs that experienced
   multiple context-switches already are relegated to the low-priority bin.

   - Location: Margo source code (``src/margo-prio-pool.c``)
   - Use case: Prioritizing critical RPCs over background work
   - Features: Thread-safe

2. **Earliest-First Pool ("earliest_first")**

   A pool that schedules work units based on timestamps, processing the earliest items
   first. Useful for time-sensitive operations.

   - Location: Margo source code (``src/margo-efirst-pool.c``)
   - Use case: Processing time-ordered events, deadline-aware scheduling
   - Features: Timestamp-based ordering, efficient for temporal scheduling

**How to Use Margo Custom Pools**:

In Bedrock or Margo configurations, you can specify these custom pool types:

.. code-block:: json

   {
       "argobots": {
           "pools": [
               {
                   "name": "rpc_pool",
                   "kind": "prio_wait",
                   "access": "mpmc"
               },
               {
                   "name": "timed_pool",
                   "kind": "earliest_first",
                   "access": "mpmc"
               }
           ]
       }
   }

Learning from Examples
----------------------

To learn how to implement custom pools and schedulers:

1. **Study Margo's implementations**:

   - Clone the Margo repository: https://github.com/mochi-hpc/mochi-margo
   - Examine ``src/margo-prio-pool.c`` and ``src/margo-efirst-pool.c``
   - Look at how they implement the ``ABT_pool_user_def`` interface

2. **Review Argobots documentation**:

   - Official API docs: https://www.argobots.org/doxygen/latest/
   - Pool definition API: ``ABT_pool_user_def``
   - Scheduler definition API: ``ABT_sched_def``

3. **Start with simpler modifications**:

   - Modify existing predefined pools/schedulers
   - Add custom logic to standard FIFO behavior
   - Gradually build up to fully custom implementations

API Overview
------------

**Pool Definition Structure**:

Custom pools implement the ``ABT_pool_user_def`` structure with callbacks for:

- Pool initialization and finalization
- Unit creation and destruction
- Push and pop operations
- Size queries and empty checks
- Optional features like unit removal, printing, etc.

**Scheduler Definition Structure**:

Custom schedulers implement the ``ABT_sched_def`` structure with callbacks for:

- Scheduler initialization and finalization
- Main scheduling loop
- Pool selection logic
- Migration decisions

When to Implement Custom Pools/Schedulers
------------------------------------------

**Consider custom implementations when**:

- Profiling shows scheduling is a bottleneck
- Your workload has specific patterns not served by predefined types
- You need application-specific scheduling policies
- Integration with external systems requires custom logic

**Use predefined pools/schedulers when**:

- Standard FIFO, priority, or work-stealing patterns suffice
- You're starting a new project (optimize later if needed)
- Maintenance cost of custom code outweighs benefits

Best Practices
--------------

**Thread Safety**:
  Ensure your custom pool properly handles the specified access mode (PRIV, SPSC, MPSC, SPMC, MPMC)

**Performance**:
  Custom implementations should be well-optimized. Poorly implemented custom pools can be slower than predefined ones.

**Testing**:
  Thoroughly test custom pools and schedulers with concurrent access patterns and edge cases.

**Documentation**:
  Document your custom pool's behavior, guarantees, and configuration options.

**Compatibility**:
  Ensure your custom pool works with Argobots' migration and work-stealing mechanisms.

Further Reading
---------------

- **Argobots Paper**: "Argobots: A Lightweight Low-Level Threading and Tasking Framework"
  for understanding design principles

- **Margo Source Code**: Real-world examples of production custom pools

- **Argobots Doxygen**: Detailed API documentation for pool and scheduler interfaces
