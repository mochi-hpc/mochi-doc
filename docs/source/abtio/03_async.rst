Non-blocking I/O operations
===========================

ABT-IO provides an API to issue I/O operations in a non-blocking
manner and wait for completion later. The following code
examplifies this feature,

.. literalinclude:: ../../../code/abtio/03_async/main.c
       :language: cpp

The full list of non-blocking functions is the following.

+--------------------+------------------------------+
| ABT-IO function    | Corresponding POSIX function |
+====================+==============================+
| abt_io_open_nb     | open                         |
+--------------------+------------------------------+
| abt_io_pwrite_nb   | pwrite                       |
+--------------------+------------------------------+
| abt_io_write_nb    | write                        |
+--------------------+------------------------------+
| abt_io_pread_nb    | pread                        |
+--------------------+------------------------------+
| abt_io_read_nb     | read                         |
+--------------------+------------------------------+
| abt_io_mkostemp_nb | mkostemp                     |
+--------------------+------------------------------+
| abt_io_unlink_nb   | unlink                       |
+--------------------+------------------------------+
| abt_io_close_nb    | close                        |
+--------------------+------------------------------+

Compared with their blocking versions, these function take
an additional parameter which is a pointer to the returned
value (will be set when the operation has completed) and
return a pointer to an :code:`abt_io_op_t` object.

The user can then use :code:`abt_io_op_wait` to wait for
the operation to complete, and :code:`abt_io_op_free` to
free the operation once completed.
