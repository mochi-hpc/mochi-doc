ULTs vs Tasklets
=================

In this tutorial, you will learn about the two types of work units in Argobots: User-Level
Threads (ULTs) and tasklets. Understanding when to use each type is crucial for achieving
optimal performance in your applications.

Prerequisites
-------------

- Completed Tutorial 01 and Tutorial 02
- Understanding of stack-based execution
- Basic knowledge of recursion

What You'll Learn
-----------------

By the end of this tutorial, you will understand:

- The fundamental differences between ULTs and tasklets
- Stack management and memory implications
- When to use ULTs vs tasklets
- How to configure ULT attributes (stack size, etc.)
- Work unit reuse with revive operations
- Performance tradeoffs

Key Concepts
------------

**User-Level Threads (ULTs)**
  ULTs are stackful work units. Each ULT has its own execution stack allocated from
  the heap. This allows ULTs to:

  - **Yield execution**: Temporarily pause and resume later
  - **Be migrated**: Move between execution streams
  - **Make recursive calls**: Stack accommodates function call depth
  - **Be suspended/resumed**: Full context switching support

  **Memory**: Each ULT allocates a stack (default 16KB-256KB depending on configuration)

**Tasklets**
  Tasklets are stackless work units. They execute using the execution stream's stack.
  This means tasklets:

  - **Run to completion**: Cannot yield or be suspended mid-execution
  - **Cannot be migrated**: Tied to their execution stream until completion
  - **Limited recursion**: Share the execution stream's stack
  - **Lower overhead**: No stack allocation needed

  **Memory**: Minimal overhead (just the work unit descriptor)

**When to Use Each**

Use **ULTs** when:
  - You need to yield (for synchronization or cooperative scheduling)
  - Work units may be migrated between execution streams
  - The function makes deep recursive calls
  - You need to suspend/resume execution
  - Example: RPC handlers, long-running computations

Use **Tasklets** when:
  - Work runs to completion without yielding
  - Function has shallow call depth
  - You're creating many work units (lower memory overhead)
  - Migration is not needed
  - Example: Simple callbacks, data-parallel tasks

ULT Example
-----------

Here's an example demonstrating ULTs with recursive calls:

.. literalinclude:: ../../../code/argobots/03_ults_tasklets/ult_example.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Custom Stack Size (lines 48-50)**
  .. code-block:: c

     ABT_thread_attr_create(&attr);
     ABT_thread_attr_set_stacksize(attr, 16384); /* 16KB stack */

  ULTs need sufficient stack for their operations. For recursive algorithms, you may
  need larger stacks. For simple operations, smaller stacks save memory.

**Recursive Computation (lines 11-15)**
  The fibonacci function makes recursive calls, requiring stack space. This works fine
  with ULTs but would be problematic with tasklets.

**Stack Memory Cost**
  Creating 8 ULTs with 16KB stacks costs 128KB of memory just for stacks. With thousands
  of ULTs, this becomes significant.

Tasklet Example
---------------

Here's an equivalent example using tasklets for simpler computations:

.. literalinclude:: ../../../code/argobots/03_ults_tasklets/tasklet_example.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Simpler Computation (lines 19-27)**
  The tasklet performs a simple calculation without deep call stacks. This is ideal
  for tasklets.

**No Attributes Needed (line 51)**
  Tasklets don't have stack configuration since they don't allocate stacks.
  ``ABT_task_create()`` has a simpler signature than ``ABT_thread_create()``.

**Lower Overhead**
  Creating thousands of tasklets uses minimal memory compared to ULTs. This makes
  tasklets ideal for fine-grained data parallelism.

Work Unit Reuse with Revive
----------------------------

Creating and destroying work units has overhead. For operations that repeat frequently,
you can reuse work units with revive operations:

.. literalinclude:: ../../../code/argobots/03_ults_tasklets/revive_example.c
   :language: c
   :linenos:

Key Points
~~~~~~~~~~

**Initial Creation (lines 35-41)**
  Work units are created normally the first time.

**Revive Instead of Create (lines 49-53)**
  .. code-block:: c

     ABT_thread_revive(pool, work_func, &thread_args[i], &threads[i]);

  Instead of freeing and creating a new ULT, we revive the existing one. This:

  - Reuses the allocated stack (for ULTs)
  - Avoids allocation/deallocation overhead
  - Maintains the work unit handle

**When to Use Revive**
  - Iterative algorithms with repeated work patterns
  - High-frequency work unit creation
  - When the number of concurrent work units is bounded
  - Pool-based task systems with worker recycling

**Performance Impact**
  Reviving can be 2-10x faster than create/destroy for high-frequency operations,
  especially for ULTs where stack allocation is expensive.

Building and Running the Examples
----------------------------------

Build all examples:

.. code-block:: bash

   cd code/argobots/03_ults_tasklets
   mkdir build && cd build
   cmake ..
   make

Run the ULT example:

.. code-block:: bash

   ./ult_example

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

Run the tasklet example:

.. code-block:: bash

   ./tasklet_example

Expected output:

.. code-block:: text

   === Tasklet Example ===
   Tasklets are stackless and execute to completion

   Tasklet 0 on ES 0: 10^2 = 100
   Tasklet 1 on ES 0: 11^2 = 121
   ...
   All tasklets completed
   Note: Tasklets have lower overhead but cannot yield or migrate

Run the revive example:

.. code-block:: bash

   ./revive_example

This will show multiple iterations reusing the same work units.

Performance Comparison
----------------------

Here's a comparison of ULTs vs tasklets:

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Aspect
     - ULT
     - Tasklet
   * - Stack
     - Allocated (16KB-256KB)
     - Uses xstream's stack
   * - Creation overhead
     - Higher (stack allocation)
     - Lower (minimal)
   * - Memory per unit
     - ~16KB + descriptor
     - ~100 bytes (descriptor only)
   * - Can yield
     - Yes
     - No
   * - Can be migrated
     - Yes (while suspended)
     - No
   * - Can be suspended
     - Yes
     - No (runs to completion)
   * - Suitable for
     - Long-running, I/O, RPC
     - Data-parallel, callbacks
   * - Max concurrent units
     - Thousands
     - Millions

**Rule of Thumb**: If you can use a tasklet, use a tasklet. Only use ULTs when you need
their specific capabilities (yielding, migration, or deep stacks).

Thread Attributes
-----------------

ULTs support various attributes for customization:

**Stack Size**
  .. code-block:: c

     ABT_thread_attr_create(&attr);
     ABT_thread_attr_set_stacksize(attr, 32768);  /* 32KB */
     ABT_thread_create(pool, func, arg, attr, &thread);
     ABT_thread_attr_free(&attr);

  Default stack size varies by platform. Query with ``ABT_thread_attr_get_stacksize()``.

**Migratable**
  .. code-block:: c

     ABT_thread_attr_set_migratable(attr, ABT_FALSE);

  Prevent ULT migration. Slightly improves performance if migration isn't needed.

**Stack Guard**
  Some implementations support stack overflow detection. Check Argobots documentation
  for your version.

Mochi Usage Patterns
---------------------

In Mochi applications:

**Margo RPC Handlers: ULTs**
  RPC handlers need to yield while waiting for I/O or other RPCs. They must be ULTs:

  .. code-block:: c

     /* Margo automatically creates ULTs for RPC handlers */
     DEFINE_MARGO_RPC_HANDLER(my_rpc_handler)
     {
         /* Can make other RPCs, which yield */
         margo_forward(some_other_rpc, ...);
     }

**Progress Loop: ULT**
  The Margo progress loop runs in a dedicated ULT to handle incoming RPCs.

**Bulk Transfer Callbacks: Tasklets or ULTs**
  Simple callbacks can be tasklets. Complex callbacks that make RPCs need to be ULTs.

**Computation Tasks: Tasklets**
  Data-parallel computations are ideal for tasklets:

  .. code-block:: c

     /* Process array elements in parallel with tasklets */
     for (int i = 0; i < n; i++) {
         ABT_task_create(pool, process_element, &data[i], NULL);
     }

Bedrock Configuration
~~~~~~~~~~~~~~~~~~~~~

Bedrock doesn't directly expose ULT vs tasklet choice (that's in your code), but pool
configuration affects work unit performance:

.. code-block:: json

   {
       "argobots": {
           "pools": [
               {
                   "name": "rpc_pool",
                   "kind": "fifo_wait",
                   "comment": "For ULT-based RPC handlers"
               },
               {
                   "name": "task_pool",
                   "kind": "fifo",
                   "access": "mpmc",
                   "comment": "For tasklet-based parallel tasks"
               }
           ]
       }
   }

Common Pitfalls
---------------

**Deep Recursion in Tasklets**
  .. code-block:: c

     /* WRONG: Deep recursion in tasklet */
     void recursive_tasklet(void *arg) {
         int n = *(int *)arg;
         if (n > 0) {
             int next = n - 1;
             ABT_task_create(pool, recursive_tasklet, &next, NULL);
         }
     }

  This creates many tasklets but each uses the xstream's stack during execution.
  Very deep recursion will overflow the stack. Use ULTs for recursive algorithms.

**Yielding in Tasklets**
  .. code-block:: c

     /* WRONG: Cannot yield in tasklets */
     void bad_tasklet(void *arg) {
         ABT_self_yield();  /* This will fail or cause undefined behavior */
     }

  Tasklets run to completion and cannot yield. If you need to yield, use a ULT.

**Excessive ULT Stack Size**
  .. code-block:: c

     /* WRONG: Unnecessarily large stack */
     ABT_thread_attr_set_stacksize(attr, 1024*1024);  /* 1MB! */

  Most ULTs don't need huge stacks. Default (16-64KB) is usually fine. Profile before
  increasing stack size.

**Not Freeing Attributes**
  .. code-block:: c

     ABT_thread_attr_create(&attr);
     ABT_thread_create(pool, func, arg, attr, &thread);
     /* WRONG: Forgot to free attribute */

  Always free attributes after use:

  .. code-block:: c

     ABT_thread_attr_free(&attr);

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

**Tasklet Functions**
  - ``int ABT_task_create(ABT_pool pool, void (*task_func)(void *), void *arg, ABT_task *newtask)``

    Create a new tasklet.

  - ``int ABT_task_revive(ABT_pool pool, void (*task_func)(void *), void *arg, ABT_task *task)``

    Revive a terminated tasklet for reuse.

  - ``int ABT_task_free(ABT_task *task)``

    Free a tasklet (blocks until termination).

  - ``int ABT_task_get_state(ABT_task task, ABT_task_state *state)``

    Query tasklet state (READY, RUNNING, TERMINATED).

**Attribute Functions**
  - ``int ABT_thread_attr_create(ABT_thread_attr *newattr)``

    Create a ULT attribute object.

  - ``int ABT_thread_attr_free(ABT_thread_attr *attr)``

    Free a ULT attribute object.

  - ``int ABT_thread_attr_set_stacksize(ABT_thread_attr attr, size_t stacksize)``

    Set the stack size for ULTs created with this attribute.

  - ``int ABT_thread_attr_set_migratable(ABT_thread_attr attr, ABT_bool migratable)``

    Control whether ULTs can be migrated between execution streams.

Next Steps
----------

Now that you understand ULTs and tasklets, you can move on to:

- **Tutorial 04: Schedulers and Work Distribution** - Learn about different scheduler
  types and how they distribute work among execution streams.

- **Tutorial 05: Barriers** - Learn synchronization primitives for coordinating multiple
  work units.

Understanding ULT vs tasklet tradeoffs is essential for optimal Mochi application
performance. Choose wisely based on your workload characteristics.
