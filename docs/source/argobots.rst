Argobots (User-level Threading)
===============================

Argobots is a lightweight, low-level threading and tasking runtime system designed
for high-performance computing (HPC) applications. It provides user-level threads
(ULTs) and tasklets as the fundamental units of work, managed by customizable
schedulers and execution streams. Argobots is a critical foundation for the Mochi
ecosystem, powering both Margo and Thallium's asynchronous execution capabilities.

Why Argobots Matters for Mochi
-------------------------------

Argobots is the execution engine behind Mochi's concurrency model:

- **Margo** uses Argobots for managing RPC handler execution and progress loops
- **Thallium** provides C++ abstractions over Margo's Argobots-based execution
- **Bedrock** allows fine-grained control over Argobots pools and execution streams
- **All Mochi services** run on Argobots threads for lightweight concurrency

Understanding Argobots is essential for:

1. **Performance tuning**: Configuring pools and schedulers for optimal throughput
2. **Resource management**: Controlling thread creation and work distribution
3. **Advanced patterns**: Implementing custom scheduling policies or work-stealing algorithms
4. **Debugging**: Understanding execution flow and identifying concurrency issues

Core Concepts
-------------

**User-Level Threads (ULTs)**
  Lightweight threads with their own stack, managed entirely in user space.
  ULTs can be migrated between execution streams and support full context switching.
  Use ULTs when you need: stackful coroutines, migration, or recursive algorithms.

**Tasklets**
  Even lighter work units without their own stack, executed to completion.
  Tasklets have lower overhead but cannot yield or be migrated during execution.
  Use tasklets for: simple computations, callbacks, or embarrassingly parallel tasks.

**Execution Streams (xstreams)**
  OS-level threads that execute Argobots work units. Each xstream has a scheduler
  that pulls work from one or more pools. Typically, you create one xstream per
  CPU core for optimal performance.

**Pools**
  Work queues that hold ULTs and tasklets waiting to be executed. Pools can be
  private to a single xstream or shared among multiple xstreams for work-stealing.
  Different pool types (FIFO, FIFO_WAIT, RANDWS) offer different synchronization
  and scheduling behaviors.

**Schedulers**
  Components that pull work units from pools and execute them on xstreams.
  Argobots provides predefined schedulers (BASIC, RANDWS, PRIO) and supports
  custom scheduler implementations for specialized policies.

When to Use Argobots Directly
------------------------------

Most Mochi users interact with Argobots indirectly through Margo or Thallium:

**Use Margo/Thallium if:**
  - You're building RPC-based microservices (most common case)
  - You want high-level abstractions for network communication
  - You're using Bedrock for deployment and configuration

**Use Argobots directly if:**
  - You're implementing data-parallel algorithms (stencil computations, etc.)
  - You need fine-grained control over work distribution and scheduling
  - You're building custom Mochi services with specialized concurrency patterns
  - You're developing custom schedulers or performance-critical components

Tutorial Roadmap
----------------

The tutorials are organized into three categories based on your needs:

**Foundation (Tutorials 01-04): Essential for All Users**
  Start here to understand basic Argobots concepts and usage patterns.
  These tutorials cover initialization, execution streams, pools, work units,
  and predefined schedulers.

**Synchronization (Tutorials 05-08): Critical for Data-Parallel Applications**
  Learn how to coordinate work units using barriers, futures, mutexes,
  condition variables, and other synchronization primitives.

**Advanced Topics (Tutorials 09-11): For Framework Developers**
  Deep dive into self operations, custom schedulers, custom pools,
  performance measurement, and debugging techniques.

Learning Paths
--------------

Choose your path based on your role:

**Mochi Application Developer** (most common)
  Tutorials: 01 → 02 → 03 → 04 → 05 → 07

  Focus on understanding how Margo/Thallium use Argobots so you can configure
  pools and schedulers effectively in Bedrock configurations.

  **Time**: ~4-6 hours

  **Outcome**: Can configure Argobots for Mochi services, understand pool access
  modes, choose appropriate schedulers

**Algorithm Implementer**
  Tutorials: 01 → 02 → 03 → 04 → 05 → 06 → 07 → 09

  Focus on implementing data-parallel and recursive algorithms using Argobots
  work units and synchronization primitives.

  **Time**: ~8-10 hours

  **Outcome**: Can implement efficient parallel algorithms, use fine-grained
  synchronization, optimize work distribution

**Framework/System Developer**
  All tutorials: 01 through 11

  Complete understanding for building Mochi services, implementing custom
  schedulers, and advanced performance tuning.

  **Time**: ~12-15 hours

  **Outcome**: Can implement custom schedulers and pools, optimize Argobots
  for specific workloads, debug complex concurrency issues

Installation
------------

Argobots is automatically installed as a dependency when you install Mochi
packages through Spack:

.. code-block:: bash

   # Install with Mochi stack
   spack install mochi-margo

   # Or install Argobots directly
   spack install argobots

For development, you can also build from source:

.. code-block:: bash

   git clone https://github.com/pmodels/argobots.git
   cd argobots
   ./autogen.sh
   ./configure --prefix=/path/to/install
   make -j
   make install

API Documentation
-----------------

The complete Argobots API reference is available at:
https://www.argobots.org/doxygen/latest/

Tutorials
---------

.. toctree::
   :maxdepth: 1

   argobots/01_intro.rst
   argobots/02_xstreams_pools.rst
   argobots/03_ults_tasklets.rst
   argobots/04_schedulers.rst
   argobots/05_barriers.rst
   argobots/06_futures.rst
   argobots/07_mutex_cond.rst
   argobots/08_sync_primitives.rst
   argobots/09_self_operations.rst
   argobots/10_custom_schedulers.rst
   argobots/11_performance_debug.rst
