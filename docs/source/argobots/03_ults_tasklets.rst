User-Level Threads (ULTs)
==========================

In this tutorial, you will learn about User-Level Threads (ULTs), the work units in Argobots
that enable lightweight parallelism and concurrency in your applications.

.. note::
   **Historical Note**: Prior to Argobots 1.2, there was a distinction between ULTs
   (User-Level Threads) and tasklets. Tasklets were stackless work units with lower
   overhead but more limitations. As of Argobots 1.2, tasklets are simply a typedef
   for ULTs, and there is no longer any functional difference. All work units are now
   ULTs with their own stacks.

Key Concepts
------------

**User-Level Threads (ULTs)**
  ULTs are the work units in Argobots. Each ULT has its own execution stack allocated
  from the heap. This allows ULTs to:

  - **Yield execution**: Temporarily pause and resume later
  - **Be migrated**: Move between execution streams
  - **Make recursive calls**: Stack accommodates function call depth
  - **Be suspended/resumed**: Full context switching support

  **Memory**: Each ULT allocates a stack (default 16KB but can be configured differently
  at compile time and at run time)

**Stack Management**
  ULTs require sufficient stack space for their operations. The stack size can be
  configured when creating ULTs:

  - Default stack size varies by platform (typically 16KB-64KB)
  - Recursive algorithms may need larger stacks
  - Simple operations can use smaller stacks to save memory
  - Stack size is set via ``ABT_thread_attr_set_stacksize()``

Basic ULT Example
-----------------

Here's a basic example demonstrating ULT creation and execution:

.. literalinclude:: ../../../code/argobots/03_ults_tasklets/ult_example.c
   :language: c
   :linenos:

Expected output:

.. code-block:: text

   === ULT Example ===
   ULTs have their own stack and can make recursive calls

   ULT 0 on ES 0: fib(10) = 55
   ULT 1 on ES 0: fib(11) = 89
   ULT 2 on ES 0: fib(12) = 144
   ...
   All ULTs completed
   Note: ULTs can yield, be suspended/resumed, and migrated

Key Points
~~~~~~~~~~

**Custom Stack Size**
  .. code-block:: c

     ABT_thread_attr_create(&attr);
     ABT_thread_attr_set_stacksize(attr, 16384); /* 16KB stack */

  ULTs need sufficient stack for their operations. For recursive algorithms, you may
  need larger stacks. For simple operations, smaller stacks save memory.

**Recursive Computation**
  The fibonacci function makes recursive calls, requiring stack space. ULTs handle
  this naturally with their own stacks.

**Stack Memory Cost**
  Creating 8 ULTs with 16KB stacks costs 128KB of memory just for stacks. With thousands
  of ULTs, consider the memory implications and adjust stack size accordingly.

.. note::

   Because RPC handlers can have deep callstacks throw networking libraries in Mochi,
   Margo will automatically set the default stack size of ULTs to 2MB. It is therefore
   recommended to use ``ABT_thread_attr_set_stacksize`` to set the stack size back
   to a smaller number if you create a ULT that you know will not require more than a
   few KB.

Lightweight Work Example
------------------------

For simple computations that don't require deep recursion, you can create ULTs with
smaller stack sizes:

.. literalinclude:: ../../../code/argobots/03_ults_tasklets/simple_example.c
   :language: c
   :linenos:

Expected output:

.. code-block:: text

   === Simple ULT Example ===
   ULTs can be used for all types of work

   ULT 0 on ES 0: 10^2 = 100
   ULT 1 on ES 0: 11^2 = 121
   ...
   All ULTs completed

Key Points
~~~~~~~~~~

**Simple Computation**
  For lightweight work that doesn't require much stack, you can still use ULTs.
  There's no need for a separate work unit type.

**Default Attributes**
  You can pass ``ABT_THREAD_ATTR_NULL`` to use default attributes, which is fine
  for most use cases.

**Performance Considerations**
  While ULTs have some overhead for stack allocation, Argobots
  optimizes this well by pre-allocating and reusing stacks.

Work Unit Reuse with Revive
----------------------------

Creating and destroying work units has overhead. For operations that repeat frequently,
you can reuse work units with revive operations:

.. literalinclude:: ../../../code/argobots/03_ults_tasklets/revive_example.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Initial Creation**
  Work units are created normally the first time.

**Revive Instead of Create**
  .. code-block:: c

     ABT_thread_revive(pool, work_func, &thread_args[i], &threads[i]);

  Instead of freeing and creating a new ULT, we revive the existing one. This:

  - Reuses the allocated stack
  - Avoids allocation/deallocation overhead
  - Maintains the work unit handle

**When to Use Revive**
  - Iterative algorithms with repeated work patterns
  - High-frequency work unit creation
  - When the number of concurrent work units is bounded
  - Pool-based task systems with worker recycling

**Performance Impact**
  Reviving can be faster than creating/destroying for high-frequency operations,
  especially when stack allocation is expensive.

Thread Attributes
-----------------

ULTs support various attributes for customization:

**Stack Size**
  .. code-block:: c

     ABT_thread_attr_create(&attr);
     ABT_thread_attr_set_stacksize(attr, 32768);  /* 32KB */
     ABT_thread_create(pool, func, arg, attr, &thread);
     ABT_thread_attr_free(&attr);

  Default stack size can be queried  with ``ABT_thread_attr_get_stacksize()``.

**Migratable**
  .. code-block:: c

     ABT_thread_attr_set_migratable(attr, ABT_FALSE);

  Prevent ULT migration. Slightly improves performance if migration isn't needed.

**Stack Guard**
  Some flags can be provided when building Argobots to support stack overflow detection. With spack,
  for instance, the ``stackguard`` variant (``none`` by default) can be set to ``canary-32``,
  ``mprotect``, or ``mprotect-strict``, for that purpose.

Mochi Usage Patterns
---------------------

In Mochi applications, ULTs are used for everything. Each RPC turns into a ULT,
and each blocking call (e.g. ``margo_forward`` yields to other ULTs so that
other work can continue while progress is made on I/O and communications).

API Reference
-------------

**ULT Functions**
  - ``int ABT_thread_create(ABT_pool pool, void (*thread_func)(void *), void *arg, ABT_thread_attr attr, ABT_thread *newthread)``

    Create a new ULT.

  - ``int ABT_thread_revive(ABT_pool pool, void (*thread_func)(void *), void *arg, ABT_thread *thread)``

    Revive a terminated ULT for reuse.

  - ``int ABT_thread_join(ABT_thread thread)``

    Wait for a ULT to terminate (doesn't free it).

  - ``int ABT_thread_free(ABT_thread *thread)``

    Join and free a ULT (blocks until termination).

  - ``int ABT_thread_yield()``

    Yield execution to allow other ULTs to run.

**Attribute Functions**
  - ``int ABT_thread_attr_create(ABT_thread_attr *newattr)``

    Create a ULT attribute object.

  - ``int ABT_thread_attr_free(ABT_thread_attr *attr)``

    Free a ULT attribute object.

  - ``int ABT_thread_attr_set_stacksize(ABT_thread_attr attr, size_t stacksize)``

    Set the stack size for ULTs created with this attribute.

  - ``int ABT_thread_attr_set_migratable(ABT_thread_attr attr, ABT_bool migratable)``

    Control whether ULTs can be migrated between execution streams.
