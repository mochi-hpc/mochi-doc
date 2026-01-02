Custom Schedulers
==================

Custom schedulers allow you to implement specialized scheduling policies beyond Argobots'
predefined schedulers. This is advanced material for framework developers building custom
Mochi services with specific scheduling requirements.

Prerequisites
-------------

- Completed Tutorials 01-09
- Strong understanding of schedulers (Tutorial 04)
- Familiarity with Argobots internals

What You'll Learn
-----------------

- Scheduler interface and lifecycle
- Implementing custom scheduling policies
- Scheduler main loop patterns
- When custom schedulers are needed

Key Concepts
------------

**Scheduler Interface**
  Custom schedulers implement the ``ABT_sched_def`` interface:
  - ``init``: Initialize scheduler data
  - ``run``: Main scheduling loop
  - ``free``: Cleanup scheduler data
  - ``get_migr_pool``: Pool for work migration (optional)

**Scheduler Main Loop**
  The heart of a scheduler:
  1. Pop work unit from pool
  2. If work available: execute it
  3. If no work: check if should stop
  4. Repeat

**When to Implement Custom Schedulers**
  - Specialized priority policies
  - Custom work distribution algorithms
  - Integration with external event loops
  - Application-specific optimizations

Custom Scheduler Example
-------------------------

Complete custom scheduler implementation:

.. literalinclude:: ../../../code/argobots/10_custom_schedulers/custom_scheduler.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Scheduler Definition (lines 67-72)**
  .. code-block:: c

     sched_def.type = ABT_SCHED_TYPE_ULT;
     sched_def.init = sched_init;
     sched_def.run = sched_run;
     sched_def.free = sched_free;

  The ``ABT_sched_def`` structure defines the scheduler callbacks.

**Init Function (lines 17-27)**
  Allocates scheduler-specific data and retrieves pools.

**Main Loop (lines 30-52)**
  - Pop work units from pool (line 36)
  - Execute with ``ABT_xstream_run_unit()`` (line 40)
  - Check for stop signal when no work (lines 44-47)
  - Call ``ABT_xstream_check_events()`` to avoid busy-wait (line 50)

**Free Function (lines 55-61)**
  Cleans up scheduler data.

Simplified Example
------------------

Minimal custom scheduler:

.. literalinclude:: ../../../code/argobots/10_custom_schedulers/simple_custom.c
   :language: c
   :linenos:

This shows the bare minimum for a working custom scheduler. No init or free needed
if you have no custom data.

Building and Running
--------------------

.. code-block:: bash

   cd code/argobots/10_custom_schedulers
   mkdir build && cd build
   cmake ..
   make
   ./custom_scheduler
   ./simple_custom

Scheduler Design Patterns
--------------------------

**Priority Scheduling**:
  .. code-block:: c

     /* Check high-priority pool first */
     ABT_pool_pop(high_priority_pool, &unit);
     if (unit == ABT_UNIT_NULL) {
         ABT_pool_pop(low_priority_pool, &unit);
     }

**Work Stealing**:
  .. code-block:: c

     /* Try own pool first, then steal from others */
     for (int i = 0; i < num_pools; i++) {
         ABT_pool_pop(pools[i], &unit);
         if (unit != ABT_UNIT_NULL) break;
     }

**Event-Driven**:
  .. code-block:: c

     /* Integrate with external event loop */
     while (!should_stop) {
         process_external_events();
         ABT_pool_pop(pool, &unit);
         if (unit != ABT_UNIT_NULL) {
             ABT_xstream_run_unit(unit, pool);
         }
     }

Custom Pools
------------

For complete control, you can also implement custom pools. This is rarely needed:

.. code-block:: c

   ABT_pool_def pool_def = {
       .p_push = my_push,
       .p_pop = my_pop,
       .p_is_empty = my_is_empty,
       /* ... other operations ... */
   };
   ABT_pool_create(&pool_def, config, &pool);

Custom pools are advanced; use predefined pools unless you have specific requirements.

When to Use Custom Schedulers
------------------------------

**Use Custom Schedulers When**:
  - Predefined schedulers don't fit your needs
  - You need application-specific scheduling policies
  - Integrating with external event loops (e.g., progress engines)
  - Building specialized frameworks on top of Argobots

**Avoid Custom Schedulers When**:
  - Predefined schedulers (BASIC, PRIO, RANDWS) suffice
  - You're building typical Mochi services (use Bedrock configs instead)
  - Complexity outweighs benefits

Best Practices
--------------

**Keep It Simple**:
  Start with simple policies. Complexity adds bugs and overhead.

**Avoid Busy-Waiting**:
  Always call ``ABT_xstream_check_events()`` when no work is available.

**Test Thoroughly**:
  Scheduler bugs can cause deadlocks, starvation, or crashes.

**Profile Performance**:
  Measure whether your custom scheduler actually improves performance.

Common Pitfalls
---------------

**Forgetting to Check Stop Signal**:
  .. code-block:: c

     while (1) {
         /* Pop and execute work */
         /* WRONG: Never checks has_to_stop */
     }

  Always check ``ABT_sched_has_to_stop()`` when idle.

**Busy-Waiting**:
  .. code-block:: c

     while (!has_to_stop) {
         ABT_pool_pop(pool, &unit);
         /* No check_events call = busy-wait */
     }

**Incorrect Unit Execution**:
  Always use ``ABT_xstream_run_unit()`` to execute work units.

API Reference
-------------

**Scheduler Functions**:
  - ``int ABT_sched_create(const ABT_sched_def *def, int num_pools, ABT_pool *pools, ABT_sched_config config, ABT_sched *newsched)``
  - ``int ABT_sched_get_pools(ABT_sched sched, int max_pools, int idx, ABT_pool *pools)``
  - ``int ABT_sched_set_data(ABT_sched sched, void *data)``
  - ``int ABT_sched_get_data(ABT_sched sched, void **data)``
  - ``int ABT_sched_has_to_stop(ABT_sched sched, ABT_bool *stop)``
  - ``int ABT_xstream_run_unit(ABT_unit unit, ABT_pool pool)``
  - ``int ABT_xstream_check_events(ABT_sched sched)``

Next Steps
----------

- **Tutorial 11: Performance and Debugging** - Learn to measure and optimize
  Argobots programs, including custom schedulers.

Custom schedulers are powerful but complex. Use them judiciously for specialized
requirements where predefined schedulers fall short.
