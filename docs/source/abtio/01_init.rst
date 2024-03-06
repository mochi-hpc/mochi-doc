Initializing ABT-IO
===================

The following code shows how to initialize and finalize ABT-IO.
ABT-IO must be initialized after Argobots and finalized before it.
The :code:`abt_io_init` function takes the number of ES to create
for handling I/O operations.

.. literalinclude:: ../../../code/abtio/01_init/main.c
       :language: cpp

More advanced tuning and configuration options can be controlled using the
:code:`abt_io_init_ext` function rather than :code:`abt_io_init`.
It accepts a json-formatted configuration specifier that supports the following parameters.

.. code-block:: c

    /* ------- abt-io configuration examples ------
     *
     * optional input field for convenience.  This will cause abt-io to create
     * an internal service pool with N execution streams in it.
     * --------------
     * {"backing_thread_count": 16}
     *
     * The user may also pass in an explicit pool.  If so, the resulting json
     * will look like this:
     * --------------
     * {"internal_pool_flag": 0}
     *
     * This is the fully resolved json description of an internal pool (may be
     * passed in explicitly, or will be generated based on the
     * backing_thread_count json parameter).
     * --------------
     * {"internal_pool_flag": 1,
     *    "internal_pool":{
     *       "kind":"fifo_wait",
     *       "access":"mpmc",
     *       "num_xstreams": 4
     *    }
     * }
     */

See :ref:`abtio_uring` for information about how to configure ABT-IO to use
io_uring to execute I/O operations.

