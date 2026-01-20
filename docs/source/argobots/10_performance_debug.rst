Performance and Debugging
==========================

This final tutorial covers performance measurement, debugging techniques, and error handling
for Argobots applications. These skills are essential for optimizing and troubleshooting
Mochi services.

Performance Measurement
-----------------------

Argobots provides timing functions for performance analysis:

.. literalinclude:: ../../../code/argobots/10_performance_debug/timing_example.c
   :language: c
   :linenos:

**Two Timing Methods**:

1. **ABT_get_wtime()** (lines 38, 51): Simple wall-clock time
   - Lightweight, minimal overhead
   - Returns current time in seconds (double)
   - Use for quick measurements

2. **ABT_timer** (lines 59-76): Timer object
   - Start, stop, read operations
   - Can be reused multiple times
   - Slightly more overhead but cleaner for repeated measurements

**Performance Metrics**:
  - Total execution time
  - Per-iteration time
  - Per-work-unit time
  - Throughput (operations/second)

Debugging with Info Functions
------------------------------

Argobots provides introspection APIs for debugging:

.. literalinclude:: ../../../code/argobots/10_performance_debug/debugging_example.c
   :language: c
   :linenos:

**Key Info Functions**:

- ``ABT_info_print_config()``: Print Argobots configuration
- ``ABT_info_print_all_xstreams()``: Print all execution streams
- ``ABT_thread_get_state()``: Query thread state

**Thread States**:
  - ``ABT_THREAD_STATE_READY``: In pool, ready to run
  - ``ABT_THREAD_STATE_RUNNING``: Currently executing
  - ``ABT_THREAD_STATE_BLOCKED``: Waiting on synchronization
  - ``ABT_THREAD_STATE_TERMINATED``: Finished execution

Error Handling
--------------

Proper error handling improves robustness:

.. literalinclude:: ../../../code/argobots/10_performance_debug/error_handling.c
   :language: c
   :linenos:

**Error Handling Pattern** (lines 9-20):
  .. code-block:: c

     int ret = ABT_some_function(...);
     if (ret != ABT_SUCCESS) {
         ABT_error_get_str(ret, err_str, &len);
         /* Handle error */
     }

**Common Error Codes**:
  - ``ABT_SUCCESS``: Operation succeeded
  - ``ABT_ERR_INV_ARG``: Invalid argument
  - ``ABT_ERR_INV_XSTREAM``: Invalid execution stream
  - ``ABT_ERR_INV_POOL``: Invalid pool
  - ``ABT_ERR_INV_THREAD``: Invalid thread
  - etc.

Common Performance Bottlenecks
-------------------------------

**1. Too Many ULTs**
  Creating millions of ULTs overwhelms the system. Solution: Batch work, or consider
  reducing stack sizes for lightweight ULTs.

**2. Fine-Grained Synchronization**
  Excessive locking/unlocking. Solution: Batch operations, use lock-free structures where possible.

**3. Poor Work Distribution**
  Load imbalance across execution streams. Solution: Use work-stealing schedulers.

**4. Excessive Context Switching**
  Too frequent yielding. Solution: Reduce yield frequency, batch work.

**5. Synchronization Overhead**
  Wrong synchronization primitive. Solution: Use appropriate primitive (eventual vs barrier vs future vs mutex...).

Debugging Strategies
--------------------

**Deadlock Debugging**:
  1. Use ``ABT_info_print_all_xstreams()`` to see thread states
  2. Check for circular dependencies in locks/futures
  3. Verify all work units can make progress

**Performance Debugging**:
  1. Measure with ``ABT_timer``
  2. Identify bottleneck operations
  3. Profile different scheduler types
  4. Check pool utilization

**Memory Debugging**:
  - Use valgrind to check for leaks
  - Ensure all ``create`` calls have matching ``free`` calls
  - Check thread-local storage cleanup

**Common Issues**:

- Forgetting to free work units: Memory leak
- Double-freeing work units: Crash
- Mismatched barrier count: Deadlock
- Not checking return values: Silent failures

Profiling Tools
---------------

**External Tools**:
  - **valgrind**: Memory leak detection
  - **perf**: CPU profiling
  - **gprof**: Call graph profiling
  - **Intel VTune**: Advanced performance analysis

**Argobots-Specific**:
  - Built-in timers for work-unit execution
  - Info functions for state inspection
  - Custom instrumentation in schedulers

Best Practices
--------------

**Performance**:
  1. Configure appropriate stack sizes for ULTs based on workload
  2. Use work-stealing for unbalanced workloads
  3. Batch small operations
  4. Minimize synchronization overhead
  5. Profile before optimizing

**Debugging**:
  1. Always check return values
  2. Use info functions during development
  3. Add assertions for invariants
  4. Test with different schedulers
  5. Start simple, add complexity gradually

**Error Handling**:
  1. Check all Argobots API return values
  2. Provide meaningful error messages
  3. Clean up resources on error paths
  4. Use helper functions for repetitive checks

API Reference
-------------

**Timing Functions**:
  - ``double ABT_get_wtime(void)``
  - ``int ABT_timer_create(ABT_timer *newtimer)``
  - ``int ABT_timer_start(ABT_timer timer)``
  - ``int ABT_timer_stop(ABT_timer timer)``
  - ``int ABT_timer_read(ABT_timer timer, double *secs)``
  - ``int ABT_timer_free(ABT_timer *timer)``

**Info Functions**:
  - ``int ABT_info_print_config(FILE *fp)``
  - ``int ABT_info_print_all_xstreams(FILE *fp)``
  - ``int ABT_info_print_xstream(FILE *fp, ABT_xstream xstream)``
  - ``int ABT_info_print_pool(FILE *fp, ABT_pool pool)``
  - ``int ABT_info_print_thread(FILE *fp, ABT_thread thread)``

**Error Functions**:
  - ``int ABT_error_get_str(int err, char *str, size_t *len)``
