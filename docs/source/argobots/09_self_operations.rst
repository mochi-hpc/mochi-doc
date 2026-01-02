Self Operations and Yielding
==============================

Self operations give ULTs direct control over their execution: yielding to the scheduler,
suspending/resuming, and explicit control flow transfers. These are advanced techniques
critical for implementing custom execution patterns and integrating with external event loops.

Prerequisites
-------------

- Completed Tutorials 01-08
- Understanding of cooperative multitasking
- Familiarity with schedulers

What You'll Learn
-----------------

- Voluntary yielding for cooperative scheduling
- Progress polling patterns (critical for Margo)
- Suspend and resume operations
- Targeted control flow transfers
- When and how to use self operations

Key Concepts
------------

**Self Operations**
  Operations that a ULT performs on itself:
  - ``ABT_self_yield()``: Voluntarily return control to scheduler
  - ``ABT_self_suspend()``: Suspend self, requiring external resume
  - ``ABT_self_exit()``: Terminate self immediately

**Cooperative Scheduling**
  Unlike preemptive scheduling (OS threads), Argobots ULTs run until they voluntarily
  yield or block. This gives predictable execution but requires cooperation.

**Progress Polling**
  Common pattern in asynchronous systems: repeatedly check for completion while yielding
  to let other work progress. Essential in Margo for network progress.

Yield-Based Synchronization
----------------------------

Yielding enables fair cooperative scheduling:

.. literalinclude:: ../../../code/argobots/09_self_operations/yield_sync.c
   :language: c
   :linenos:

**Key Points**:
  - ``ABT_self_yield()`` (line 31): Voluntarily gives up CPU
  - Scheduler selects next ULT to run
  - Yielding ULT remains runnable, will be rescheduled
  - Enables fair sharing without preemption

**Use Cases**:
  - Long-running computations that should share CPU
  - Cooperative critical sections
  - Polling loops

Progress Polling Pattern
-------------------------

Critical pattern for Margo/Mochi network progress:

.. literalinclude:: ../../../code/argobots/09_self_operations/progress_polling.c
   :language: c
   :linenos:

**Key Points**:
  - Poll for completion in loop (lines 42-52)
  - Yield after each check (line 55)
  - Non-blocking: doesn't wait, just checks
  - Lets other ULTs (like background workers) make progress

**Margo Usage**:
  .. code-block:: c

     /* Typical Margo progress pattern */
     while (!request_completed) {
         margo_progress(mid, 0);  /* Non-blocking progress */
         ABT_self_yield();        /* Let other work run */
     }

Other Self Operations
---------------------

**Suspend and Resume**:
  .. code-block:: c

     /* ULT A */
     ABT_self_suspend();  /* Suspends self, requires external resume */

     /* ULT B or external thread */
     ABT_thread_resume(thread_handle_of_A);  /* Resume A */

  Use for event-driven execution where external events trigger continuation.

**Exit**:
  .. code-block:: c

     ABT_self_exit();  /* Terminate self immediately */

  Cleaner than returning from function for early termination.

**Yield To**:
  .. code-block:: c

     ABT_self_yield_to(target_thread);  /* Yield and schedule target next */

  Direct control flow transfer. Use sparingly; breaks scheduler abstraction.

Building and Running
--------------------

.. code-block:: bash

   cd code/argobots/09_self_operations
   mkdir build && cd build
   cmake ..
   make
   ./yield_sync
   ./progress_polling

When to Use Self Operations
----------------------------

**Use ABT_self_yield() when**:
  - Implementing long-running computations
  - Progress polling loops
  - Cooperative fairness needed
  - You want to give other work a chance to run

**Use ABT_self_suspend/resume() when**:
  - Implementing event-driven execution
  - External events (I/O, signals) control execution
  - Building custom synchronization primitives

**Avoid self operations when**:
  - Normal blocking operations suffice (mutex, cond, future)
  - Not performance-critical
  - You want scheduler to make decisions

Performance Implications
------------------------

**Yielding Overhead**:
  - ``ABT_self_yield()`` has context switch overhead
  - Don't yield in tight loops
  - Balance between fairness and overhead

**Progress Polling**:
  - Good: Allows concurrent progress
  - Bad: Burns CPU if polling too frequently
  - Best practice: Yield after each poll

Common Pitfalls
---------------

**Infinite Yield Loop**:
  .. code-block:: c

     /* Bad: Yields forever with no progress */
     while (!condition) {
         ABT_self_yield();
     }

  Ensure some ULT can make progress towards the condition!

**Forgetting to Yield in Polling**:
  .. code-block:: c

     /* Bad: Busy-wait, starves other ULTs */
     while (!ready) {
         /* No yield - monopolizes CPU */
     }

  Always yield in polling loops.

**Suspending Without Resume Path**:
  .. code-block:: c

     ABT_self_suspend();  /* Who will resume us? */

  Ensure there's a clear path for resume, or ULT hangs forever.

API Reference
-------------

**Self Operations**:
  - ``int ABT_self_yield()``

    Voluntarily yield to scheduler. Returns when rescheduled.

  - ``int ABT_self_yield_to(ABT_thread thread)``

    Yield and request specific ULT be scheduled next.

  - ``int ABT_self_suspend()``

    Suspend self. Requires ``ABT_thread_resume()`` from another ULT.

  - ``int ABT_self_exit()``

    Terminate self immediately.

  - ``int ABT_self_schedule(ABT_thread thread, ABT_pool pool)``

    Schedule another ULT to a pool while running.

**Related Operations**:
  - ``int ABT_thread_resume(ABT_thread thread)``

    Resume a suspended ULT.

Next Steps
----------

- **Tutorial 10: Custom Schedulers** - Implement your own scheduling policies
  using the knowledge from this tutorial.

- **Tutorial 11: Performance and Debugging** - Measure and optimize Argobots programs.

Self operations are powerful tools for advanced Argobots programming, especially
for integrating with external event loops and implementing custom execution patterns.
