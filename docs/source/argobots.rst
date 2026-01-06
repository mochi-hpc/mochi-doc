Argobots (User-level Threading)
===============================

Argobots is a lightweight, low-level threading and tasking runtime system designed
for high-performance computing (HPC) applications. It provides user-level threads
(ULTs) as the fundamental units of work, managed by customizable schedulers and
execution streams. Argobots is a critical foundation for the Mochi ecosystem,
powering both Margo and Thallium's asynchronous execution capabilities.

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

While Margo and Thallium almost completely hide their use of Mercury,
Argobots is still central to the development of Mochi services. Users
should at the very least know how to use its synchronization mechanisms
(e.g. :code:`ABT_mutex`, :code:`ABT_cond`, :code:`ABT_eventual`, etc.),
and we recommend getting a good understanding of Argobots so you can also
split your work into ULTs as appropriate.

Core Concepts
-------------

**User-Level Threads (ULTs)**
  Lightweight threads with their own stack, managed entirely in user space.
  ULTs can be migrated between execution streams and support full context switching.

**Tasklets (deprecated as of Argobots 1.2)**
  Prior to Argobots 1.2, tasklets were stackless work units that ran to completion
  without the ability to yield or migrate. As of Argobots 1.2, tasklets are simply
  a typedef for ULTs. The ``ABT_task`` API is maintained for backward compatibility
  but should not be used in new code. All work units are now ULTs.

**Execution Streams (xstreams)**
  OS-level threads that execute Argobots work units. Each xstream has a scheduler
  that pulls work from one or more pools. Typically, you create one xstream per
  CPU core for optimal performance.

**Pools**
  Work queues that hold ULTs waiting to be executed. Pools can be private to a
  single xstream or shared among multiple xstreams for work-stealing. Different
  pool types (FIFO, FIFO_WAIT, RANDWS) offer different synchronization and
  scheduling behaviors.

**Schedulers**
  Components that pull work units from pools and execute them on xstreams.
  Argobots provides predefined schedulers (BASIC, RANDWS, PRIO) and supports
  custom scheduler implementations for specialized policies. Each xstream
  is associated with its own scheduler.

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
   argobots/05_mutex_cond.rst
   argobots/06_eventuals_rwlocks.rst
   argobots/07_barriers_futures.rst
   argobots/08_self_operations.rst
   argobots/09_custom_schedulers_pools.rst
   argobots/10_performance_debug.rst
