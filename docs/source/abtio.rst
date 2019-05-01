ABT-IO
======

Executing I/O operations (:code:`read`, :code:`write`, etc.) from an Argobots execution
stream blocks this ES until the I/O operation has completed. This is
problematic if the ES is used to run a Mercury progress loop, or to execute Mercury RPC
handlers, since it prevents any progress on network activities or the execution of
other RPC handlers. ABT-IO was developed specifically to address this problem.
It creates one or more ES to which I/O operations can be pushed, hence blocking these
ES on I/O activities rather than the ES the operations originate from.
This section gives a list of tutorials on how to use ABT-IO.

.. toctree::
   :maxdepth: 1

   abtio/01_init.rst
   abtio/02_io.rst
   abtio/03_async.rst
