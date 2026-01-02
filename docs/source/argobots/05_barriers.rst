Barriers
=========

In this tutorial, you will learn about barriers, a synchronization primitive that allows
multiple work units to wait for each other at specific points in execution. Barriers are
essential for bulk-synchronous parallel algorithms and iterative computations.

Prerequisites
-------------

- Completed Tutorials 01-04
- Understanding of parallel algorithm patterns
- Familiarity with data dependencies in parallel code

What You'll Learn
-----------------

- How barriers synchronize multiple work units
- Bulk-synchronous parallel programming model
- Using barriers in iterative algorithms
- Barrier reinitialization for multiple algorithm runs
- Performance implications of barriers

Key Concepts
------------

**Barriers**
  A barrier is a synchronization point where multiple work units wait for each other.
  When a work unit reaches a barrier, it blocks until all other work units also reach
  the barrier. Once all work units arrive, they all proceed together.

  **Think of it like**: A group of hikers who agree to wait at certain checkpoints
  until everyone arrives before continuing.

**Barrier Count**
  When creating a barrier, you specify how many work units must arrive before the
  barrier releases. This count is fixed when the barrier is created.

**Bulk-Synchronous Parallel (BSP) Model**
  Barriers enable the BSP programming model:
  1. Parallel computation phase
  2. Barrier synchronization
  3. Communication/data exchange phase
  4. Barrier synchronization
  5. Repeat

**Use Cases**:
  - Stencil computations (iterative PDE solvers)
  - Synchronous iterative algorithms
  - Phase-based simulations
  - Parallel sorting algorithms
  - Matrix operations

Stencil Example
---------------

Stencil computations update each array element based on its neighbors. This requires
synchronization to ensure all threads read the same consistent array state:

.. literalinclude:: ../../../code/argobots/05_barriers/stencil_barrier.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Two Barriers per Iteration (lines 36, 44)**
  .. code-block:: c

     ABT_barrier_wait(work->barrier);  /* After computation */
     ABT_barrier_wait(work->barrier);  /* After copying */

  We need two barriers:
  1. After computation: Ensure all threads finished computing before anyone copies
  2. After copying: Ensure all threads finished copying before next iteration

  **Why two barriers?**: Without the second barrier, some threads might start the next
  iteration's computation while others are still copying, reading inconsistent data.

**Barrier Creation (line 81)**
  .. code-block:: c

     ABT_barrier_create(NUM_THREADS, &barrier);

  The barrier is created for ``NUM_THREADS`` threads. Exactly this many threads must
  call ``ABT_barrier_wait()`` for the barrier to release.

**Data Dependencies**
  Barriers enforce happens-before relationships:
  - All computations happen before any copying
  - All copying happens before next iteration's computations

Iterative Algorithm Example
----------------------------

For algorithms with multiple runs, you can reuse barriers with reinit:

.. literalinclude:: ../../../code/argobots/05_barriers/iterative_algorithm.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Barrier Reinit (line 74)**
  .. code-block:: c

     ABT_barrier_reinit(barrier, NUM_THREADS);

  Instead of destroying and recreating the barrier, we reinitialize it. This:
  - Resets the barrier's internal state
  - More efficient than create/destroy
  - Useful for multiple algorithm runs

**Multiple Barriers per Work Unit**
  Each work unit may wait on the same barrier multiple times (lines 27, 33). The barrier
  synchronizes all threads at each wait point.

Building and Running
--------------------

.. code-block:: bash

   cd code/argobots/05_barriers
   mkdir build && cd build
   cmake ..
   make
   ./stencil_barrier
   ./iterative_algorithm

Expected output shows synchronization at each iteration.

When to Use Barriers
--------------------

**Use Barriers When**:
  - All work units must complete a phase before proceeding
  - Data dependencies span all work units
  - You need bulk-synchronous execution
  - Implementing iterative algorithms with global synchronization

**Avoid Barriers When**:
  - Dependencies are fine-grained (use futures instead - Tutorial 06)
  - Not all work units need to synchronize
  - Work units have highly variable execution times (barriers cause idle time)
  - You need more flexible synchronization (use condition variables - Tutorial 07)

Performance Implications
------------------------

**Synchronization Overhead**
  Barriers have overhead:
  - Each ``ABT_barrier_wait()`` requires atomic operations
  - All work units must wait for the slowest one
  - Can cause load imbalance if work is uneven

**Load Balancing**
  Barriers expose load imbalance:
  - If one thread takes 2x longer, all threads wait
  - Consider work-stealing to balance load before barriers
  - Or use finer-grained synchronization (futures)

**Cache Effects**
  Barrier synchronization can have cache benefits:
  - Ensures all writes are visible before reads
  - Can help with cache coherence on NUMA systems

Best Practices
--------------

**Match Barrier Count to Work Units**
  .. code-block:: c

     /* Create barrier */
     ABT_barrier_create(NUM_THREADS, &barrier);

     /* Create exactly NUM_THREADS work units */
     for (int i = 0; i < NUM_THREADS; i++) {
         /* Pass barrier to each work unit */
     }

  Mismatched counts cause deadlock or incorrect behavior.

**One Barrier per Synchronization Point**
  Don't reuse the same barrier for different synchronization purposes:

  .. code-block:: c

     /* Good: Separate barriers for different purposes */
     ABT_barrier computation_barrier;
     ABT_barrier io_barrier;

     /* Bad: Reusing same barrier for different sync points risks confusion */

**Reinit for Reuse**
  .. code-block:: c

     /* Reinit between algorithm runs */
     for (int run = 0; run < NUM_RUNS; run++) {
         /* ... run algorithm ... */
         ABT_barrier_reinit(barrier, NUM_THREADS);
     }

Common Pitfalls
---------------

**Wrong Barrier Count**
  .. code-block:: c

     /* WRONG: Barrier count doesn't match thread count */
     ABT_barrier_create(NUM_THREADS - 1, &barrier);
     for (int i = 0; i < NUM_THREADS; i++) {
         /* Last thread will wait forever */
     }

**Forgetting to Free Barriers**
  .. code-block:: c

     ABT_barrier_create(n, &barrier);
     /* ... use barrier ... */
     ABT_barrier_free(&barrier);  /* Don't forget! */

**Nested Barriers with Same Work Units**
  .. code-block:: c

     /* WRONG: Deadlock if same work units hit both barriers */
     ABT_barrier_wait(barrier1);
     ABT_barrier_wait(barrier2);
     /* If execution order varies, this can deadlock */

  Ensure barrier ordering is consistent across all work units.

**Barrier in Recursive Code**
  Barriers are problematic in recursive algorithms:

  .. code-block:: c

     /* BAD: Recursive depth varies, barrier count unclear */
     void recursive_func(void *arg) {
         if (depth > 0) {
             create_child_threads();
             ABT_barrier_wait(barrier);  /* How many threads? */
             recursive_func(depth - 1);
         }
     }

  Use futures (Tutorial 06) for recursive synchronization.

API Reference
-------------

**Barrier Functions**
  - ``int ABT_barrier_create(uint32_t num_waiters, ABT_barrier *newbarrier)``

    Create a barrier for ``num_waiters`` work units.

  - ``int ABT_barrier_wait(ABT_barrier barrier)``

    Wait at the barrier. Blocks until all ``num_waiters`` work units call wait.

  - ``int ABT_barrier_reinit(ABT_barrier barrier, uint32_t num_waiters)``

    Reinitialize a barrier for reuse. Resets internal state and waiter count.

  - ``int ABT_barrier_free(ABT_barrier *barrier)``

    Free a barrier. Must not be called while work units are waiting.

Next Steps
----------

- **Tutorial 06: Futures** - Learn fine-grained dependency management for algorithms
  where barriers are too coarse.

- **Tutorial 07: Mutexes and Condition Variables** - Learn more flexible synchronization
  primitives for complex coordination patterns.

Barriers are a fundamental tool for bulk-synchronous parallel algorithms in Mochi
applications, especially for data-parallel computations.
