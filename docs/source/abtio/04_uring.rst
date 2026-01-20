.. _abtio_uring:

Using io_uring with ABT-IO
==========================

Liburing support can be enabled at compile time by adding the
:code:`+liburing` variant to the mochi-abt-io spack package build or by
adding :code:`--enable-liburing` to the configure arguments if compiling by
hand.

Once ABT-IO has been compiled with liburing support, it can be enabled at run time by setting "num_urings" json configuration parameter to a
value of 1 or higher.  ABT-IO will associate a dedicated internal execution
stream with each uring to handle submission and completion of operations.
If "num_urings" is > 1, then uring operations will be issued in round-robin
fashion across the rings.  Any ABT-IO operations that are not supported
by uring are serviced as usual by the normal ABT-IO pool.

Optional uring flags can also be specified as follows:

.. code-block:: json

   {
       "liburing_flags": [
           "IOSQE_ASYNC",
           "IORING_SETUP_SQPOLL",
           "IORING_SETUP_COOP_TASKRUN",
           "IORING_SETUP_SINGLE_ISSUER",
           "IORING_SETUP_DEFER_TASKRUN"
       ]
   }

Please refer to the liburing documentation for an explanation of these
flags.

Recommended configuration
-------------------------

If you are using liburing then we recommend setting :code:`"num_urings":1`
and :code:`"liburing_flags":["IOSQE_ASYNC"]`, and configuring the normal
ABT-IO pool (used for any oprations not supported by liburing) to have just
one execution stream.  For example:

.. code-block:: json

   {
       "internal_pool_flag": 1,
       "internal_pool": {
           "kind": "fifo_wait",
           "access": "mpmc",
           "num_xstreams": 1
       },
       "num_urings": 1,
       "liburing_flags": ["IOSQE_ASYNC"]
   }

One uring is sufficient to saturate most storage devices as long as the
kernel is instructed to use asynchronous mode by default so that the
execution stream can put as many concurrent operations in flight as
possible.  Note that the "IOSQE_ASYNC" flag is not supported on older Linux
kernels, however.  One execution stream is also typicallly sufficient for
the internal pool as it is only used to service a limited set of non-I/O
operations (i.e., not the :code:`read` or :code:`write` operations).  You
may find it helpful to increase this pool size if you anticipate servicing a
large number of such operations, however.

