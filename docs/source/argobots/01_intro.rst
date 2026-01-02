Introduction and Basic Execution
==================================

In this tutorial, you will learn the fundamentals of Argobots by creating and
executing your first user-level thread (ULT). This is the foundation for all
Argobots programs and will introduce you to the basic lifecycle of an Argobots
application.

Prerequisites
-------------

- Basic knowledge of C programming
- Familiarity with threads and concurrency concepts (helpful but not required)
- Argobots installed (see installation section below)

What You'll Learn
-----------------

By the end of this tutorial, you will understand:

- How to initialize and finalize Argobots
- The concept of execution streams (xstreams)
- How to create, execute, and wait for user-level threads (ULTs)
- The basic structure of an Argobots application
- The lifecycle of Argobots work units

Key Concepts
------------

Before diving into the code, let's understand the fundamental concepts:

**Initialization and Finalization**
  Every Argobots program begins with ``ABT_init()`` and ends with ``ABT_finalize()``.
  Initialization sets up the Argobots runtime and creates a primary execution stream.
  Finalization cleans up all resources and ensures proper shutdown.

**Execution Streams (xstreams)**
  An execution stream is an OS-level thread that executes Argobots work units.
  When you call ``ABT_init()``, Argobots automatically creates a primary execution
  stream that runs on the main thread. You can create additional execution streams
  for parallel execution (covered in Tutorial 02).

**User-Level Threads (ULTs)**
  ULTs are lightweight threads managed entirely in user space by Argobots. Unlike
  OS threads (pthreads), ULTs have very low creation overhead and can be created
  in large numbers. Each ULT executes a function you provide.

**Pools**
  Pools are work queues that hold ULTs waiting to be executed. Each execution stream
  has at least one pool. When you create a ULT, you specify which pool it should be
  added to. The execution stream's scheduler pulls ULTs from the pool and executes them.

Basic Example: Hello World
---------------------------

Let's start with a simple example that creates a single ULT:

.. literalinclude:: ../../../code/argobots/01_intro/hello_world.c
   :language: c
   :linenos:

Code Walkthrough
----------------

Let's examine each step of the program:

**Step 1: Initialization (line 24)**
  .. code-block:: c

     ABT_init(argc, argv);

  This initializes the Argobots runtime system. It:

  - Sets up internal data structures
  - Creates a primary execution stream
  - Creates a default pool for the primary execution stream
  - Converts the main thread into an Argobots execution stream

**Step 2: Get the Primary Execution Stream (line 28)**
  .. code-block:: c

     ABT_xstream_self(&xstream);

  This retrieves a handle to the current execution stream (in this case, the primary
  execution stream created by ``ABT_init()``). We need this handle to access the
  execution stream's pool.

**Step 3: Get the Main Pool (line 32)**
  .. code-block:: c

     ABT_xstream_get_main_pools(xstream, 1, &pool);

  This retrieves the default pool associated with the execution stream. The second
  parameter (1) specifies we want to retrieve one pool. Each execution stream can
  have multiple pools; we're getting the main pool here.

**Step 4: Create a ULT (lines 35-36)**
  .. code-block:: c

     ABT_thread_create(pool, hello_world_fn, &thread_arg,
                       ABT_THREAD_ATTR_NULL, &thread);

  This creates a ULT with the following parameters:

  - ``pool``: The pool where the ULT will be added
  - ``hello_world_fn``: The function to execute
  - ``&thread_arg``: Argument passed to the function
  - ``ABT_THREAD_ATTR_NULL``: Use default attributes (stack size, etc.)
  - ``&thread``: Output handle for the created ULT

  Note that this is a **non-blocking** operation. The ULT is created and added to
  the pool, but it may not execute immediately.

**Step 5: Wait for Completion (line 40)**
  .. code-block:: c

     ABT_thread_free(&thread);

  This is a critical operation. ``ABT_thread_free()`` does two things:

  1. **Joins** the ULT (waits for it to complete if it hasn't already)
  2. **Frees** the resources associated with the ULT

  This is a **blocking** operation - it will not return until the ULT has finished
  executing. If you don't call this, the ULT might still be running when the program
  exits, leading to undefined behavior.

**Step 6: Finalization (line 44)**
  .. code-block:: c

     ABT_finalize();

  This shuts down the Argobots runtime:

  - Waits for all work units to complete
  - Destroys all execution streams, pools, and schedulers
  - Frees all internal resources

  After calling ``ABT_finalize()``, you cannot use any Argobots functions.

Building and Running the Example
---------------------------------

To build the example, first ensure Argobots is installed:

.. code-block:: bash

   # Using Spack
   spack install argobots

Then build the example using CMake:

.. code-block:: bash

   cd code/argobots/01_intro
   mkdir build && cd build
   cmake ..
   make
   ./hello_world

Expected Output
~~~~~~~~~~~~~~~

When you run the program, you should see output similar to:

.. code-block:: text

   Argobots initialized
   Got primary execution stream
   Got main pool
   Created ULT
   Hello from ULT 1!
   ULT completed and freed
   Argobots finalized

The exact order of messages may vary slightly depending on when the ULT executes,
but the "Hello from ULT 1!" message will always appear between "Created ULT" and
"ULT completed and freed".

Understanding the Execution Flow
---------------------------------

Here's what happens when you run this program:

1. ``ABT_init()`` creates a primary execution stream with a scheduler and pool
2. The main function runs on this primary execution stream
3. ``ABT_thread_create()`` creates a ULT and adds it to the pool
4. The ULT is scheduled for execution (may happen immediately or later)
5. ``ABT_thread_free()`` blocks until the ULT completes
6. The ULT function (``hello_world_fn``) executes and returns
7. ``ABT_thread_free()`` returns, freeing ULT resources
8. ``ABT_finalize()`` shuts down the Argobots runtime

Important Notes
~~~~~~~~~~~~~~~

**Non-Blocking Creation**
  Creating a ULT with ``ABT_thread_create()`` does not block. The ULT is added to
  the pool and will be executed when the scheduler decides. However, in this simple
  example with a single execution stream, the scheduler often executes the ULT
  immediately when we call ``ABT_thread_free()``.

**Implicit Progress**
  Certain Argobots operations (like ``ABT_thread_free()``) trigger implicit progress
  of the scheduler. This means the scheduler may execute pending work units even
  though you didn't explicitly call a yield or scheduling function.

**The Primary Execution Stream**
  The primary execution stream is special - it's created automatically by ``ABT_init()``
  and represents the main thread of your program. You cannot free or join the primary
  execution stream; it's automatically cleaned up by ``ABT_finalize()``.

Common Pitfalls
---------------

**Forgetting to Free ULTs**
  Always call ``ABT_thread_free()`` for every ULT you create. If you forget, the ULT
  may still be running when the program exits, causing crashes or resource leaks.

  .. code-block:: c

     /* WRONG: ULT never freed */
     ABT_thread_create(pool, my_func, arg, ABT_THREAD_ATTR_NULL, &thread);
     /* ... no ABT_thread_free() ... */

**Using Handles After Free**
  After calling ``ABT_thread_free()``, the thread handle becomes invalid. Don't try
  to use it again:

  .. code-block:: c

     ABT_thread_free(&thread);
     ABT_thread_free(&thread);  /* WRONG: double free */

**Calling Argobots Functions After Finalize**
  Once you call ``ABT_finalize()``, the Argobots runtime is shut down. Any Argobots
  function calls after this point will fail or crash.

API Reference
-------------

This tutorial covered the following Argobots functions:

**Initialization and Finalization**
  - ``int ABT_init(int argc, char **argv)``

    Initialize the Argobots runtime. Must be called before any other Argobots function.

  - ``int ABT_finalize(void)``

    Finalize the Argobots runtime. Must be called to properly shut down Argobots.

**Execution Stream Functions**
  - ``int ABT_xstream_self(ABT_xstream *xstream)``

    Get a handle to the current execution stream.

  - ``int ABT_xstream_get_main_pools(ABT_xstream xstream, int max_pools, ABT_pool *pools)``

    Retrieve the main pools associated with an execution stream.

**ULT Functions**
  - ``int ABT_thread_create(ABT_pool pool, void (*thread_func)(void *), void *arg, ABT_thread_attr attr, ABT_thread *newthread)``

    Create a new ULT and add it to the specified pool.

  - ``int ABT_thread_free(ABT_thread *thread)``

    Join (wait for completion) and free a ULT. This is a blocking operation.

**Special Constants**
  - ``ABT_THREAD_ATTR_NULL``

    Use default attributes when creating a ULT (stack size, etc.)

Next Steps
----------

Now that you understand the basics of Argobots execution, you can move on to:

- **Tutorial 02: Execution Streams and Pools** - Learn how to create multiple execution
  streams for parallel execution and understand different pool types.

- **Tutorial 03: ULTs vs Tasklets** - Understand the differences between ULTs and
  tasklets, and when to use each.

For more details on the Argobots API, see the official documentation at:
https://www.argobots.org/doxygen/latest/
