Barriers and Futures
====================

In this tutorial, you will learn about two important synchronization primitives for
parallel algorithms: barriers for bulk-synchronous patterns and futures for fine-grained
dependency management.

Understanding the Synchronization Spectrum
-------------------------------------------

These primitives form a spectrum from coarse-grained to fine-grained synchronization:

**Barrier** (all-to-all, N-to-N):
  - **N work units** all synchronize with each other
  - Everyone waits for everyone
  - Use for: bulk-synchronous algorithms, phase transitions

**Future** (many-to-one, N-to-1):
  - **N producers** each set one compartment
  - **One consumer** waits for all N compartments
  - Use for: gathering results from multiple workers, parallel reduction

**Eventual** (one-to-many, 1-to-N):
  - **One producer** sets a value
  - **Multiple consumers** can wait for that value
  - Use for: broadcast pattern (Tutorial 06)

Barriers
--------

A barrier is a synchronization point where multiple work units wait for each other.
When a work unit reaches a barrier, it blocks until all other work units also reach
the barrier. Once all work units arrive, they all proceed together.

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

Stencil Example with Barriers
------------------------------

Stencil computations update each array element based on its neighbors. This requires
synchronization to ensure all threads read the same consistent array state:

.. literalinclude:: ../../../code/argobots/07_barriers_futures/stencil_barrier.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Two Barriers per Iteration**
  .. code-block:: c

     ABT_barrier_wait(work->barrier);  /* After computation */
     ABT_barrier_wait(work->barrier);  /* After copying */

  We need two barriers:
  1. After computation: Ensure all threads finished computing before anyone copies
  2. After copying: Ensure all threads finished copying before next iteration

  **Why two barriers?**: Without the second barrier, some threads might start the next
  iteration's computation while others are still copying, reading inconsistent data.

**Barrier Creation**
  .. code-block:: c

     ABT_barrier_create(NUM_THREADS, &barrier);

  The barrier is created for ``NUM_THREADS`` threads. Exactly this many threads must
  call ``ABT_barrier_wait()`` for the barrier to release.

**Data Dependencies**
  Barriers enforce happens-before relationships:
  - All computations happen before any copying
  - All copying happens before next iteration's computations

**Multiple Barriers per Work Unit**
  Each work unit may wait on the same barrier multiple times. The barrier
  synchronizes all threads at each wait point.

**Reinitialization**

  ``ABT_barrier_reinit`` may be used to re-initialize a barrier with a different
  number of waiters. There is no need to reinitialize a barrier to wait multiple
  times on it with the same number of waiters.

Futures
-------

**Futures provide multiple-producer-single-consumer synchronization.** Multiple work units
(producers) can contribute values to compartments of a future, and a single consumer waits
for all compartments to be filled.

You can think of a future as an eventuel with multiple values (compartments), however
the future does not store the values. It stores pointers to them, which means it is
up to the caller to ensure the pointed memory remains valid. Also, ``ABT_future_wait``,
which blocks until all the compartments are filled, does not return the contained
values. Instead, a callback provided so ``ABT_future_create`` is invoked when
all the compartments are filled.

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

  The callback is invoked automatically when all compartments are set, before
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

.. literalinclude:: ../../../code/argobots/07_barriers_futures/parallel_reduce.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Multiple Producers**
  .. code-block:: c

     ABT_future_create(NUM_WORKERS, reduction_callback, &result_future);

     for (int i = 0; i < NUM_WORKERS; i++) {
         /* Each worker sets one compartment */
         ABT_thread_create(pool, worker_func, &worker_args[i], ...);
     }

  The future has one compartment per worker. This is the multiple-producer pattern.

**Callback Receives All Values**
  .. code-block:: c

     void reduction_callback(void **args) {
         for (int i = 0; i < NUM_WORKERS; i++) {
             int *partial = (int *)args[i];
             total += *partial;
         }
     }

  The callback is invoked when all workers complete. ``args[]`` contains all the
  pointers passed via ``ABT_future_set()``.

**Single Consumer Waits**
  .. code-block:: c

     ABT_future_wait(result_future);

  Main thread (single consumer) blocks until all NUM_WORKERS compartments are set.

**Persistent Storage Required**
  .. code-block:: c

     int partial_results[NUM_WORKERS];  /* Must outlive workers */

  Values passed to ``ABT_future_set()`` must remain valid until the callback executes.
  Stack variables in worker functions will be destroyed too early.

When to Use Futures
-------------------

**Use Futures When**:
  - Multiple producers, one consumer (gather/reduce pattern)
  - Sparse dependency graphs (not all-to-all)
  - Each consumer waits for a specific subset of producers
  - You want to expose maximum parallelism

**Use Eventuals When**:
  - Single producer, single or multiple consumers
  - Simpler one-to-many broadcast (Tutorial 06)

**Use Barriers When**:
  - All work units must synchronize (N-to-N)
  - Bulk-synchronous parallel algorithms
  - Phase-based computation

Common Patterns
---------------

**Parallel Reduction (Futures)**
  .. code-block:: c

     /* N workers compute partial results */
     ABT_future_create(N, reduction_callback, &future);
     for (i = 0; i < N; i++) {
         ABT_thread_create(pool, worker, &args[i], ...);
     }
     ABT_future_wait(future);  /* Wait for all N */

**Bulk-Synchronous Iteration (Barriers)**
  .. code-block:: c

     ABT_barrier_create(N, &barrier);
     for (i = 0; i < N; i++) {
         ABT_thread_create(pool, worker, ...);
     }
     /* Workers do: compute(), barrier_wait(), communicate(), barrier_wait() */

**Divide and Conquer (Futures)**
  .. code-block:: c

     /* Fork into N subproblems */
     ABT_future_create(N, merge_callback, &future);
     for (i = 0; i < N; i++) {
         ABT_thread_create(pool, subproblem, &args[i], ...);
     }
     ABT_future_wait(future);  /* Wait for all subproblems */

Common Pitfalls
---------------

**Wrong Barrier Count**
  .. code-block:: c

     /* WRONG: Barrier count doesn't match thread count */
     ABT_barrier_create(NUM_THREADS - 1, &barrier);
     for (int i = 0; i < NUM_THREADS; i++) {
         /* Last thread will wait forever */
     }

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
     /* Waiter blocks forever! Needs 4 sets */
     ABT_future_wait(future);

  Number of compartments must match number of producers.

**Forgetting to Free Resources**
  .. code-block:: c

     ABT_barrier_create(n, &barrier);
     /* ... use barrier ... */
     ABT_barrier_free(&barrier);  /* Don't forget! */

     ABT_future_create(n, callback, &future);
     /* ... use future ... */
     ABT_future_free(&future);  /* Don't forget! */

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
