Eventuals and Reader-Writer Locks
===================================

This tutorial covers two specialized synchronization primitives: eventuals for single-write
broadcast scenarios, and reader-writer locks for read-heavy workloads.

Eventuals
---------

Eventuals are simpler than a combination of mutex and condition variable for
scenarios where one of more consumers wait on a value from a single producer.

.. literalinclude:: ../../../code/argobots/06_eventuals_rwlocks/eventual_example.c
   :language: c
   :linenos:

**Key Properties**:
- ``ABT_eventual_wait()`` can be called by multiple ULTs
- All waiting ULTs are unblocked when ``ABT_eventual_set()`` is called
- The value remains available for subsequent waits
- Simpler than futures (explained later)  when you don't need compartments

.. note::

   If the eventual is used only as a synchronization mechanism with no attached
   value, a static version of it (``ABT_eventual_memory``) may be used, in a way
   similar to ``ABT_mutex_memory``.

Reader-Writer Locks
-------------------

Reader-writer locks allow multiple concurrent readers but exclusive writers:

.. literalinclude:: ../../../code/argobots/06_eventuals_rwlocks/rwlock_example.c
   :language: c
   :linenos:

**Key Points**:
- Multiple readers can hold read lock simultaneously
- Writers get exclusive access (no readers or other writers)
- Ideal for read-heavy workloads

**Use Cases**:
- Configuration data (frequent reads, rare updates)
- Lookup tables and caches
- Shared metadata
- Directory structures

**Performance**:
Reader-writer locks have overhead. Use only when:
- Reads vastly outnumber writes
- Critical sections are long enough to amortize locking overhead
- Otherwise, use regular mutexes

Common Pitfalls
---------------

**Using Eventuals for Multiple Writers**
  .. code-block:: c

     /* WRONG: Multiple threads calling ABT_eventual_set() */
     void worker(void *arg) {
         ABT_eventual_set(eventual, &value);  /* Race condition! */
     }

  Eventuals support only one ``set()`` call. For multiple producers, use futures.

**Using RWLocks for Write-Heavy Workloads**

  If writes are common, regular mutexes are usually faster.

**Forgetting to Free Resources**
  .. code-block:: c

     ABT_eventual_create(sizeof(int), &eventual);
     /* ... use it ... */
     ABT_eventual_free(&eventual);  /* Don't forget! */

     ABT_rwlock_create(&rwlock);
     /* ... use it ... */
     ABT_rwlock_free(&rwlock);  /* Don't forget! */

API Reference
-------------

**Eventual Functions**:
  - ``int ABT_eventual_create(int nbytes, ABT_eventual *neweventual)``

    Create an eventual for a value of ``nbytes`` size.

  - ``int ABT_eventual_wait(ABT_eventual eventual, void **value)``

    Wait for the eventual to be set. Multiple ULTs can wait.
    ``value`` is set to point to the eventual's stored value.

  - ``int ABT_eventual_test(ABT_eventual eventual, void **value, int *is_ready)``

    Non-blocking test if eventual is ready.

  - ``int ABT_eventual_set(ABT_eventual eventual, void *value, int nbytes)``

    Set the eventual's value, unblocking all waiters.
    Can only be called once per eventual.

  - ``int ABT_eventual_free(ABT_eventual *eventual)``

    Free an eventual object.

**Reader-Writer Lock Functions**:
  - ``int ABT_rwlock_create(ABT_rwlock *newrwlock)``

    Create a reader-writer lock.

  - ``int ABT_rwlock_rdlock(ABT_rwlock rwlock)``

    Acquire read lock. Multiple readers can hold this simultaneously.

  - ``int ABT_rwlock_wrlock(ABT_rwlock rwlock)``

    Acquire write lock. Exclusive access (blocks all readers and writers).

  - ``int ABT_rwlock_unlock(ABT_rwlock rwlock)``

    Release read or write lock.

  - ``int ABT_rwlock_free(ABT_rwlock *rwlock)``

    Free a reader-writer lock.
