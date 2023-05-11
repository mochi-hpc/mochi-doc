Using Mochi in conjunction with other threading libraries
=========================================================

Users should be especially careful when using Margo libraries
in conjunction with other threading libraries.
As explained in the previous tutorial about MPI, Margo uses
Argobots internally for threading. Argobots provides user-level threads
(ULT), which have to explicitely yield to one another in order to
context-switch. Blocking on an Argobots mutex, lock, eventual, or future
will also produce a context-switch to another ULT, allowing resource
sharing.

Users relying on another threading library in conjunction with the
Mochi library must keep in mind that blocking calls in these libraries
will not make Argobots yield to other ULTs. These calls will block
the entire execution stream from which they are made, preventing
further progress altogether.

It is recommended, when using another threading library with Argobots,
to isolate calls for this library in a dedicated ES, or to use dedicated
ES for all Mochi-related work. For example, using a dedicated progress
ES along with dedicated RPC handler ES when initializing Margo, and making
sure RPC handlers don't make calls to the second threading library,
is a good way to prevent any conflict from happening.
