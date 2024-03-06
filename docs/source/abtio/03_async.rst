Non-blocking I/O operations
===========================

ABT-IO provides an API to issue I/O operations in a non-blocking
manner and wait for completion later. The following code
examplifies this feature,

.. literalinclude:: ../../../code/abtio/03_async/main.c
       :language: cpp

Every ABT-IO function described in :ref:`abtio_issuing` has a corresponding non-blocking variant. These
non-blocking variants take an additional parameter which is a pointer to the
returned value (will be set when the operation has completed) and return a
pointer to an :code:`abt_io_op_t` object.

The user can then use :code:`abt_io_op_wait` to wait for
the operation to complete, and :code:`abt_io_op_free` to
free the operation once completed.
