ABT-IO
======

ABT-IO is a Mochi library that provides wrappers for conventional POSIX I/O
functions.  The wrappers are prefixed with :code:`abt_io_` as in
:code:`abt_io_open()`, :code:`abt_io_read()`, and :code:`abt_io_write()`.
These wrapper functions have the same arguments and semantics as their POSIX
counterparts, except that the underlying I/O operations are offloaded to
internal resources to be executed asynchronously.  ABT-IO presents both a
blocking API and a non-blocking API to callers.  In the former case, the
calling thread is suspended so that other user-level threads may
gracefully execute until the I/O operation is complete.

Rationale
---------

Executing I/O operations (:code:`read()`, :code:`write()`, etc.) directly
from within an Argobots user-level thread (ULT) is technically legal but
will block the underlying execution stream (ES) until the I/O operation has
completed. This is problematic (e.g., it may limit concurrency, produce excessive
latency, or even cause deadlock) if the ES is expected to execute other
cooperatively-scheduled code. In Mochi for example, an ES is likely to be
responsible for executing multiple RPC handlers or communication progress
loops, and those capabilities will be starved while the ES is blocked
awaiting completion of an I/O system call.

ABT-IO was developed specifically to address this problem.  It creates one
or more decoupled execution streams to which I/O operations are delegated so
they can be executed there on behalf of the caller without perturbing the
caller's user-level thread scheduling.  This also enables I/O concurrency to
be tuned independently from the caller's level of execution concurrency.

ABT-IO may also optionally be configured to use
`io_uring <https://kernel.dk/io_uring.pdf>`_ on supported platforms to
further accelerate I/O performance.

References
----------

A description of the original implementation of ABT-IO can be found in
Section 5.3 of `S.
Seo et al., "Argobots: A Lightweight Low-Level Threading and Tasking
Framework," in IEEE Transactions on Parallel and Distributed Systems, vol.
29, no. 3, pp. 512-526, 1 March 2018, doi: 10.1109/TPDS.2017.2766062.
<https://ieeexplore.ieee.org/document/8082139>`_


ABT-IO Usage
------------

This section gives a list of tutorials on how to use ABT-IO.

.. toctree::
   :maxdepth: 1

   abtio/01_init.rst
   abtio/02_io.rst
   abtio/03_async.rst
   abtio/04_uring.rst
