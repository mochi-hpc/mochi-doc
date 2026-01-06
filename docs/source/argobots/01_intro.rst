Introduction and Basic Execution
==================================

In this tutorial, you will learn the fundamentals of Argobots by creating and
executing your first user-level thread (ULT). This is the foundation for all
Argobots programs and will introduce you to the basic lifecycle of an Argobots
application.

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

**Step 1: Initialization**
  .. code-block:: c

     ABT_init(argc, argv);

  This initializes the Argobots runtime system. It:

  - Sets up internal data structures
  - Creates a primary execution stream
  - Creates a default pool for the primary execution stream
  - Converts the main thread into an Argobots execution stream

**Step 2: Get the Primary Execution Stream**
  .. code-block:: c

     ABT_xstream_self(&xstream);

  This retrieves a handle to the current execution stream (in this case, the primary
  execution stream created by ``ABT_init()``). We need this handle to access the
  execution stream's pool.

**Step 3: Get the Main Pool**
  .. code-block:: c

     ABT_xstream_get_main_pools(xstream, 1, &pool);

  This retrieves the default pool associated with the execution stream. The second
  parameter (1) specifies we want to retrieve one pool. Each execution stream can
  have multiple pools; we're getting the main pool here.

**Step 4: Create a ULT**
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

**Step 5: Wait for Completion**
  .. code-block:: c

     ABT_thread_join(thread);

  This is a **blocking** operation that will not return until the ULT has finished
  executing.


**Step 6: Free the ULT**
  .. code-block:: c

     ABT_thread_free(&thread);

  **Frees** the resources associated with the ULT.

**Step 6: Finalization**
  .. code-block:: c

     ABT_finalize();

  This shuts down the Argobots runtime:

  - Waits for all work units to complete
  - Destroys all execution streams, pools, and schedulers
  - Frees all internal resources

  After calling ``ABT_finalize()``, you cannot use any Argobots functions.

Expected Output
~~~~~~~~~~~~~~~

When you run the program, you should see output similar to:

.. code-block:: text

   Argobots initialized
   Got primary execution stream
   Got main pool
   Created ULT
   Hello from ULT 1!
   ULT completed
   ULT freed
   Argobots finalized

The exact order of messages may vary slightly depending on when the ULT executes,
but the "Hello from ULT 1!" message will always appear before "ULT completed".

Important Notes
~~~~~~~~~~~~~~~

**Non-Blocking Creation**
  Creating a ULT with ``ABT_thread_create()`` does not block. The ULT is added to
  the pool and will be executed when the scheduler decides. In this simple
  example with a single execution stream, the scheduler will execute the ULT
  when we call ``ABT_thread_join()``.

**Implicit Progress**
  Certain Argobots operations (like ``ABT_thread_join()``) trigger implicit progress
  of the scheduler. This means the scheduler may execute pending work units even
  though you didn't explicitly call a yield or scheduling function.

**The Primary Execution Stream**
  The primary execution stream is special - it's created automatically by ``ABT_init()``
  and represents the main thread of your program. You cannot free or join the primary
  execution stream; it is automatically cleaned up by ``ABT_finalize()``.

.. note::

   Passing :code:`NULL` instead of :code:`&thread` as the 5th argument to
   :code:`ABT_thread_create` will create an anonymous ULT. Such a ULT will
   automatically be freed once it has finished executing.

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

  - ``int ABT_thread_join(ABT_thread thread)``

    Join (wait for completion of) the ULT. This is a blocking operation.

  - ``int ABT_thread_free(ABT_thread *thread)``

    Free a ULT.

**Special Constants**
  - ``ABT_THREAD_ATTR_NULL``

    Use default attributes when creating a ULT (stack size, etc.)
