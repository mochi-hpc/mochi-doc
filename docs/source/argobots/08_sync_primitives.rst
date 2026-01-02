Other Synchronization Primitives
==================================

This tutorial covers additional synchronization primitives: eventuals for single-write
scenarios, reader-writer locks for read-heavy workloads, and work-unit keys for thread-local
storage.

Prerequisites
-------------

- Completed Tutorials 01-07
- Understanding of futures and mutexes

What You'll Learn
-----------------

- Eventuals for simple single-value synchronization
- Reader-writer locks for concurrent reads
- Work-unit keys for thread-local storage
- When to use each primitive

Eventuals
---------

Eventuals are simpler than futures for single-write, multiple-read scenarios:

.. literalinclude:: ../../../code/argobots/08_sync_primitives/eventual_example.c
   :language: c
   :linenos:

**Eventuals vs Futures**:
- Eventuals: One writer, multiple readers (no compartments)
- Futures: Multiple compartments, can be reset
- Eventuals are simpler and more efficient for single-write cases

**Use Cases**:
- Broadcast a computed result to many consumers
- Initialization values
- Configuration data

Reader-Writer Locks
-------------------

Reader-writer locks allow multiple concurrent readers but exclusive writers:

.. literalinclude:: ../../../code/argobots/08_sync_primitives/rwlock_example.c
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

**Performance**:
Reader-writer locks have overhead. Use only when:
- Reads vastly outnumber writes (90%+ reads)
- Critical sections are long enough to amortize locking overhead
- Otherwise, use regular mutexes

Work-Unit Keys (Thread-Local Storage)
--------------------------------------

Keys provide thread-local storage for ULTs:

.. literalinclude:: ../../../code/argobots/08_sync_primitives/thread_local.c
   :language: c
   :linenos:

**Key Points**:
- Each ULT can store its own value for a key
- Useful for per-thread state without passing arguments
- Similar to pthread_key_t

**Use Cases**:
- Per-thread caches
- Thread-local random number generators
- Error codes or context information

**Memory Management**:
You're responsible for allocating/freeing key values. Use destructor function in
``ABT_key_create()`` for automatic cleanup.

Building and Running
--------------------

.. code-block:: bash

   cd code/argobots/08_sync_primitives
   mkdir build && cd build
   cmake ..
   make
   ./eventual_example
   ./rwlock_example
   ./thread_local

Choosing Synchronization Primitives
------------------------------------

**Barrier**:
  All-to-all synchronization, bulk-synchronous algorithms

**Future**:
  Fine-grained dependencies, multiple producers

**Eventual**:
  Single producer, multiple consumers, broadcast pattern

**Mutex + Cond**:
  General-purpose, producer-consumer, complex conditions

**Reader-Writer Lock**:
  Read-heavy workloads with rare updates

**Work-Unit Keys**:
  Per-thread data storage

API Reference
-------------

**Eventual Functions**:
  - ``int ABT_eventual_create(int nbytes, ABT_eventual *neweventual)``
  - ``int ABT_eventual_wait(ABT_eventual eventual, void **value)``
  - ``int ABT_eventual_test(ABT_eventual eventual, void **value, int *is_ready)``
  - ``int ABT_eventual_set(ABT_eventual eventual, void *value, int nbytes)``
  - ``int ABT_eventual_free(ABT_eventual *eventual)``

**Reader-Writer Lock Functions**:
  - ``int ABT_rwlock_create(ABT_rwlock *newrwlock)``
  - ``int ABT_rwlock_rdlock(ABT_rwlock rwlock)``
  - ``int ABT_rwlock_wrlock(ABT_rwlock rwlock)``
  - ``int ABT_rwlock_unlock(ABT_rwlock rwlock)``
  - ``int ABT_rwlock_free(ABT_rwlock *rwlock)``

**Key Functions**:
  - ``int ABT_key_create(void (*destructor)(void *value), ABT_key *newkey)``
  - ``int ABT_key_set(ABT_key key, void *value)``
  - ``int ABT_key_get(ABT_key key, void **value)``
  - ``int ABT_key_free(ABT_key *key)``

Next Steps
----------

- **Tutorial 09: Self Operations** - Direct control over ULT execution with yields
  and suspension.

- **Tutorial 10: Custom Schedulers** - Build your own scheduling policies.

These primitives complete your Argobots synchronization toolkit.
