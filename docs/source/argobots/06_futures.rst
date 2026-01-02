Futures and Fine-Grained Dependencies
=======================================

**Futures provide multiple-producer-single-consumer synchronization.** Multiple work units
(producers) can contribute values to compartments of a future, and a single consumer waits
for all compartments to be filled. This makes futures sit between eventuals (single-producer
-single-consumer) and barriers (multiple-producer-multiple-consumer).

Prerequisites
-------------

- Completed Tutorials 01-05
- Understanding of data dependencies
- Familiarity with barriers and their limitations

What You'll Learn
-----------------

- Multiple-producer-single-consumer synchronization pattern
- How futures enable fine-grained synchronization
- Future compartments for gathering values from multiple producers
- Expressing dependency graphs with futures
- When to use futures vs. eventuals vs. barriers

Synchronization Primitive Spectrum
-----------------------------------

Understanding where futures fit in the synchronization landscape:

**Eventual** (single-producer-single-consumer):
  - One producer sets a value
  - One consumer waits for that value
  - Use for: simple producer-consumer pairs

**Future** (multiple-producer-single-consumer):
  - **N producers** each set one compartment
  - **One consumer** waits for all N compartments
  - Use for: gathering results from multiple workers, parallel reduction

**Barrier** (multiple-producer-multiple-consumer):
  - **N work units** all synchronize with each other
  - Everyone waits for everyone
  - Use for: bulk-synchronous algorithms, phase transitions

**Key Insight**: Futures are for scenarios where you need to wait for multiple producers
but don't need all producers to wait for each other.

Key Concepts
------------

**Compartments**
  A future with N compartments requires N ``ABT_future_set()`` calls before the consumer
  is unblocked. Each producer sets exactly one compartment.

  .. code-block:: c

     /* Create future with 4 compartments (4 producers) */
     ABT_future_create(4, callback, &future);

     /* Worker 0 sets compartment 0 */
     ABT_future_set(future, &result0);

     /* Worker 1 sets compartment 1 */
     ABT_future_set(future, &result1);

     /* ... workers 2 and 3 ... */

     /* Consumer waits for all 4 */
     ABT_future_wait(future);  /* Blocks until all 4 compartments set */

**Callback for Value Retrieval**
  The **only** way to retrieve values from a future is via the callback provided to
  ``ABT_future_create()``. There is no ``ABT_future_get()`` function.

  .. code-block:: c

     void gather_callback(void **args) {
         /* args[i] is the pointer passed by i-th ABT_future_set() */
         int *val0 = (int *)args[0];
         int *val1 = (int *)args[1];
         /* ... process values ... */
     }

     ABT_future_create(num_compartments, gather_callback, &future);

  The callback is invoked automatically when all compartments are set, just before
  ``ABT_future_wait()`` returns.

**Futures vs Barriers**
  - **Barriers**: All work units wait for all others (N-to-N synchronization)
  - **Futures**: One consumer waits for N producers (N-to-1 synchronization)

  Futures expose more parallelism when only one work unit needs to wait for results,
  while producers can immediately move on to other work.

Parallel Reduction with Futures
--------------------------------

This example demonstrates the core use case: multiple workers computing partial results,
one coordinator gathering them.

.. literalinclude:: ../../../code/argobots/06_futures/parallel_reduce.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Multiple Producers (lines 95-103)**
  .. code-block:: c

     ABT_future_create(NUM_WORKERS, reduction_callback, &result_future);

     for (int i = 0; i < NUM_WORKERS; i++) {
         /* Each worker sets one compartment */
         ABT_thread_create(pool, worker_func, &worker_args[i], ...);
     }

  The future has one compartment per worker. This is the multiple-producer pattern.

**Callback Receives All Values (lines 28-37)**
  .. code-block:: c

     void reduction_callback(void **args) {
         for (int i = 0; i < NUM_WORKERS; i++) {
             int *partial = (int *)args[i];
             total += *partial;
         }
     }

  The callback is invoked when all workers complete. ``args[]`` contains all the
  pointers passed via ``ABT_future_set()``.

**Single Consumer Waits (lines 109-111)**
  .. code-block:: c

     ABT_future_wait(result_future);

  Main thread (single consumer) blocks until all NUM_WORKERS compartments are set.

**Persistent Storage Required (line 22)**
  .. code-block:: c

     int partial_results[NUM_WORKERS];  /* Must outlive workers */

  Values passed to ``ABT_future_set()`` must remain valid until the callback executes.
  Stack variables in worker functions will be destroyed too early.

Stencil with Futures
---------------------

Futures also enable fine-grained synchronization in iterative algorithms. Each work unit
waits only for its specific dependencies, not all work units.

.. literalinclude:: ../../../code/argobots/06_futures/stencil_future.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Sparse Dependencies**
  Each cell waits only for its neighbors (2-4 cells), not all cells. This exposes more
  parallelism than barriers.

**Future Set and Reset (throughout)**
  .. code-block:: c

     ABT_future_set(neighbor_future, NULL);  /* Signal completion */
     ABT_future_reset(future);                /* Prepare for next iteration */

  Futures can be reused across iterations with reset.

**Better Parallelism**
  - With barriers: All cells wait for the slowest cell
  - With futures: Fast cells can run ahead, limited only by data dependencies

Building and Running
--------------------

.. code-block:: bash

   cd code/argobots/06_futures
   mkdir build && cd build
   cmake ..
   make
   ./parallel_reduce
   ./stencil_future

Observe how futures coordinate multiple producers with one consumer.

When to Use Futures
-------------------

**Use Futures When**:
  - Multiple producers, one consumer (gather/reduce pattern)
  - Sparse dependency graphs (not all-to-all)
  - Each consumer waits for a specific subset of producers
  - You want to expose maximum parallelism

**Use Eventuals When**:
  - Single producer, single consumer
  - Simpler one-to-one synchronization

**Use Barriers When**:
  - All work units must synchronize (N-to-N)
  - Bulk-synchronous parallel algorithms
  - Phase-based computation

**DON'T Use Futures For**:
  - Single-producer-single-consumer (use eventual instead)
  - All-to-all synchronization (use barrier instead)

Common Patterns
---------------

**Parallel Reduction**
  .. code-block:: c

     /* N workers compute partial results */
     ABT_future_create(N, reduction_callback, &future);
     for (i = 0; i < N; i++) {
         ABT_thread_create(pool, worker, &args[i], ...);
     }
     ABT_future_wait(future);  /* Wait for all N */

**Divide and Conquer**
  .. code-block:: c

     /* Fork into N subproblems */
     ABT_future_create(N, merge_callback, &future);
     for (i = 0; i < N; i++) {
         ABT_thread_create(pool, subproblem, &args[i], ...);
     }
     ABT_future_wait(future);  /* Wait for all subproblems */

**Neighbor Synchronization (Stencils)**
  .. code-block:: c

     /* Wait for left and right neighbors */
     ABT_future_create(2, NULL, &future);
     /* Neighbors call ABT_future_set() when ready */
     ABT_future_wait(future);  /* Proceed when both ready */

Common Pitfalls
---------------

**No ABT_future_get() Function**
  .. code-block:: c

     /* WRONG: This function does not exist! */
     int *value;
     ABT_future_get(future, &value);  /* Compilation error */

  The **only** way to get values is via the callback provided to ``ABT_future_create()``.

**Forgetting to Reset Futures**
  .. code-block:: c

     /* WRONG: Reusing future without reset */
     ABT_future_set(future, &value);
     /* ... later in next iteration ... */
     ABT_future_set(future, &value);  /* Error! Already set */

  Always reset between uses:

  .. code-block:: c

     ABT_future_set(future, &value);
     ABT_future_wait(future);
     ABT_future_reset(future);  /* Reset for next use */

**Mismatched Compartment Count**
  .. code-block:: c

     /* Create with 4 compartments */
     ABT_future_create(4, callback, &future);

     /* Only 3 workers set compartments */
     /* Worker 0 */ ABT_future_set(future, &val0);
     /* Worker 1 */ ABT_future_set(future, &val1);
     /* Worker 2 */ ABT_future_set(future, &val2);
     /* Worker 3 never calls set! */

     /* Waiter blocks forever! Needs 4 sets */
     ABT_future_wait(future);

  Number of compartments must match number of producers.

**Stack Variables in Callback**
  .. code-block:: c

     void bad_worker(void *arg) {
         int local_result = 42;
         ABT_future_set(future, &local_result);  /* WRONG! */
         /* local_result destroyed when function returns */
     }

     /* Callback accesses dangling pointer */
     void callback(void **args) {
         int *val = (int *)args[0];  /* Undefined behavior */
     }

  Use heap-allocated or static storage for values passed to futures.

**Circular Dependencies**
  .. code-block:: c

     /* Thread A waits for future B */
     ABT_future_wait(futureB);

     /* Thread B waits for future A */
     ABT_future_wait(futureA);

     /* Deadlock! */

  Design dependency graphs to be acyclic (DAG).

API Reference
-------------

**Future Functions**
  - ``int ABT_future_create(uint32_t compartments, void (*cb_func)(void **arg), ABT_future *newfuture)``

    Create a future with specified number of compartments.
    ``cb_func`` is invoked when all compartments are set, receiving array of value pointers.
    Pass ``NULL`` for ``cb_func`` if you only need synchronization without value gathering.

  - ``int ABT_future_wait(ABT_future future)``

    Wait for all compartments to be set. Blocks until ready.
    Callback (if provided) is invoked just before this returns.

  - ``int ABT_future_test(ABT_future future, ABT_bool *flag)``

    Non-blocking test if future is ready (all compartments set).

  - ``int ABT_future_set(ABT_future future, void *value)``

    Set one compartment with a value pointer. Sets the next unfilled compartment.
    If this is the last compartment, wakes waiters and invokes callback.

  - ``int ABT_future_reset(ABT_future future)``

    Reset future for reuse. All compartments become unfilled again.

  - ``int ABT_future_free(ABT_future *future)``

    Free a future object.

**Important**: There is **no** ``ABT_future_get()`` function. Values are retrieved only
via the callback.

Next Steps
----------

- **Tutorial 07: Mutexes and Condition Variables** - Learn traditional locking
  primitives for protecting shared data.

- **Tutorial 08: Other Synchronization Primitives** - Explore eventuals (single-producer
  -single-consumer), reader-writer locks, and work-unit keys.

Futures are the right tool for multiple-producer-single-consumer patterns like parallel
reduction, divide-and-conquer, and gathering results from multiple workers.
