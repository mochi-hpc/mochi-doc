Mutexes and Condition Variables
=================================

Mutexes and condition variables are traditional synchronization primitives for protecting
shared data and coordinating work units. They are essential for thread-safe data structures
and complex coordination patterns.

Prerequisites
-------------

- Completed Tutorials 01-06
- Understanding of race conditions and data races
- Familiarity with critical sections

What You'll Learn
-----------------

- Using mutexes to protect shared data
- Condition variables for wait/signal patterns
- Producer-consumer implementations
- Building thread-safe data structures
- Pthread interoperability (critical for Mochi/MPI integration)
- Mutex priority levels

Key Concepts
------------

**Mutexes (Mutual Exclusion)**
  A mutex ensures only one work unit accesses a critical section at a time.
  - ``ABT_mutex_lock()``: Acquire exclusive access (blocks if held by another)
  - ``ABT_mutex_unlock()``: Release access
  - Protected region: Code between lock and unlock

**Condition Variables**
  Condition variables allow work units to wait for specific conditions.
  - ``ABT_cond_wait(mutex)``: Release mutex, wait for signal, reacquire mutex
  - ``ABT_cond_signal()``: Wake one waiter
  - ``ABT_cond_broadcast()``: Wake all waiters

**Critical Pattern**:
  .. code-block:: c

     ABT_mutex_lock(mutex);
     while (!condition) {
         ABT_cond_wait(cond, mutex);  /* Atomically unlocks mutex and waits */
     }
     /* Condition is now true, mutex is locked */
     ABT_mutex_unlock(mutex);

**Why the while loop?**: Spurious wakeups and broadcast signals mean the condition
might not be true when you wake up. Always recheck.

Producer-Consumer Pattern
--------------------------

The classic producer-consumer pattern uses mutex and condition variables for coordination:

.. literalinclude:: ../../../code/argobots/07_mutex_cond/producer_consumer.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Protecting Shared State (lines 35, 51)**
  All accesses to the shared buffer are protected by mutex lock/unlock.

**Waiting on Conditions (lines 38-40, 66-68)**
  .. code-block:: c

     while (buf->count == BUFFER_SIZE) {
         ABT_cond_wait(buf->not_full, buf->mutex);
     }

  Producer waits while buffer is full. ``ABT_cond_wait()`` atomically releases the
  mutex and blocks. When signaled, it reacquires the mutex before returning.

**Signaling (lines 49, 78)**
  After producing/consuming, signal the opposite side that the condition changed.

Thread-Safe Queue
-----------------

Building reusable thread-safe data structures with mutexes:

.. literalinclude:: ../../../code/argobots/07_mutex_cond/shared_queue.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Encapsulated Locking (lines 32-43, 48-61)**
  Lock/unlock happens inside queue operations. Users don't need to know about the mutex.

**Short Critical Sections**
  Mutex is held only during the actual queue manipulation, not during application logic.

**Yielding on Busy (lines 82, 95)**
  When queue is full/empty, yield to let other work units run before retrying.

Pthread Interoperability
-------------------------

Critical for Mochi: Argobots mutexes work with pthreads (needed for MPI integration):

.. literalinclude:: ../../../code/argobots/07_mutex_cond/pthread_interop.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Shared Synchronization (lines 44-53)**
  Pthreads can lock Argobots mutexes and wait on Argobots condition variables.
  This is essential when mixing Mochi services with pthread-based libraries (like MPI).

**Common Use Case in Mochi**:
  - Margo RPC handlers (ULTs) and MPI communication (pthreads) sharing data
  - Progress threads (pthreads) signaling Argobots work units
  - Integrating Mochi with pthread-based I/O libraries

Building and Running
--------------------

.. code-block:: bash

   cd code/argobots/07_mutex_cond
   mkdir build && cd build
   cmake ..
   make
   ./producer_consumer
   ./shared_queue
   ./pthread_interop

Mutex Priority Levels
----------------------

Argobots mutexes support priority levels for scheduling:

.. code-block:: c

   ABT_mutex_lock_high(mutex);    /* High priority lock */
   ABT_mutex_lock_low(mutex);     /* Low priority lock */
   ABT_mutex_spinlock(mutex);     /* Spin instead of blocking */

Use priority locks when you need to influence scheduling decisions based on critical section importance.

Best Practices
--------------

**Keep Critical Sections Short**
  .. code-block:: c

     /* Bad: Long critical section */
     ABT_mutex_lock(mutex);
     expensive_computation();  /* Don't do this under lock */
     shared_data = result;
     ABT_mutex_unlock(mutex);

     /* Good: Minimal critical section */
     result = expensive_computation();
     ABT_mutex_lock(mutex);
     shared_data = result;
     ABT_mutex_unlock(mutex);

**Always Use while for Condition Variables**
  .. code-block:: c

     /* Wrong */
     if (!ready) {
         ABT_cond_wait(cond, mutex);
     }

     /* Right */
     while (!ready) {
         ABT_cond_wait(cond, mutex);
     }

**Unlock in Reverse Order**
  If you lock multiple mutexes, unlock in reverse order:

  .. code-block:: c

     ABT_mutex_lock(mutex1);
     ABT_mutex_lock(mutex2);
     /* ... */
     ABT_mutex_unlock(mutex2);
     ABT_mutex_unlock(mutex1);

**Signal vs Broadcast**
  - ``ABT_cond_signal()``: Wake one waiter (efficient for single consumer)
  - ``ABT_cond_broadcast()``: Wake all waiters (needed when condition affects all)

Common Pitfalls
---------------

**Deadlock**
  .. code-block:: c

     /* Thread A */
     ABT_mutex_lock(mutex1);
     ABT_mutex_lock(mutex2);

     /* Thread B */
     ABT_mutex_lock(mutex2);  /* Locks in opposite order */
     ABT_mutex_lock(mutex1);  /* Deadlock! */

  **Fix**: Always lock mutexes in the same order.

**Forgetting to Unlock**
  .. code-block:: c

     ABT_mutex_lock(mutex);
     if (error) {
         return;  /* Wrong! Mutex still locked */
     }
     ABT_mutex_unlock(mutex);

  **Fix**: Use error handling that ensures unlock, or use RAII-style wrappers.

**Not Holding Mutex During cond_wait**
  .. code-block:: c

     /* Wrong */
     ABT_cond_wait(cond, mutex);  /* Mutex not locked! */

  ``ABT_cond_wait()`` requires the mutex to be locked when called.

**Using if Instead of while**
  Spurious wakeups and broadcast signals mean you must recheck the condition:

  .. code-block:: c

     /* Wrong */
     ABT_mutex_lock(mutex);
     if (!ready) {
         ABT_cond_wait(cond, mutex);
     }
     /* ready might still be false! */

API Reference
-------------

**Mutex Functions**
  - ``int ABT_mutex_create(ABT_mutex *newmutex)``
  - ``int ABT_mutex_lock(ABT_mutex mutex)``
  - ``int ABT_mutex_trylock(ABT_mutex mutex)``
  - ``int ABT_mutex_spinlock(ABT_mutex mutex)``
  - ``int ABT_mutex_lock_high(ABT_mutex mutex)``
  - ``int ABT_mutex_lock_low(ABT_mutex mutex)``
  - ``int ABT_mutex_unlock(ABT_mutex mutex)``
  - ``int ABT_mutex_free(ABT_mutex *mutex)``

**Condition Variable Functions**
  - ``int ABT_cond_create(ABT_cond *newcond)``
  - ``int ABT_cond_wait(ABT_cond cond, ABT_mutex mutex)``
  - ``int ABT_cond_timedwait(ABT_cond cond, ABT_mutex mutex, const struct timespec *abstime)``
  - ``int ABT_cond_signal(ABT_cond cond)``
  - ``int ABT_cond_broadcast(ABT_cond cond)``
  - ``int ABT_cond_free(ABT_cond *cond)``

**Static Initializers**
  - ``ABT_MUTEX_INITIALIZER``: Static mutex initialization
  - ``ABT_COND_INITIALIZER``: Static condition variable initialization

Next Steps
----------

- **Tutorial 08: Other Synchronization Primitives** - Eventuals, reader-writer locks,
  and work-unit keys.

- **Tutorial 09: Self Operations** - Yielding, suspension, and direct control flow.

Mutexes and condition variables are fundamental for building thread-safe Mochi services
that integrate with pthread-based libraries like MPI.
