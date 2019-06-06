Using Mochi in conjunction with MPI
===================================

There is no problem using Mochi libraries alongside MPI.
However, some precautions must be taken. If you are using
MPI in conjunction with either Thallium or Margo, and witness
your code hanging, this page might be for you. This is a
common mistake was have seen countless time. In the following,
we will take Margo as an example, although the same applies
to Thallium.

Understanding *where* the progress loop is running
--------------------------------------------------

Margo internally uses Argobots to manage user-level threads (ULT).
ULTs are scheduled on execution streams (ES). Contrary to a preemptive
multithreading system like pthreads, ULTs running on the same ES must
explicitely yield to one another in order for each ULT to get a chance
to run. Yielding happens either explicitely or when calling an Argobots
function that will block the ULT (e.g., trying to lock a mutex). If a
ULT blocks on a function that is not Argobots-aware, it will not yield
to other ULT in its ES, which will block the entire ES.

In Margo, the Mercury progress loop is put in its own ULT.
When initializing Margo using :code:`margo_init`, the third argument
indicates *where* this ULT will be placed: a value of 0 indicates that
it will be running in the context of the ES that called :code:`margo_init`.
A value of 1 will make Margo create a new ES dedicated to running
the progress loop.

Understanding *when* the progress loop is running
-------------------------------------------------

In a client program, if the progress loop doesn't have its own ES,
it will execute whenever :code:`margo_forward`, :code:`margo_provider_forward`,
or :code:`margo_wait` are called. These are the functions that need to wait
for the completion of a Mercury operation before returning. Hence, they
have to run the progress loop until such completion happens.
If the client is also using MPI, this generally does not
pose any problem because the client will either block on an MPI call
(and not need the progress loop to be running) or block on a Margo call
(and not need MPI progress to happen).

If the progress loop has been placed in a dedicated ES, it will running
continuously in the background and, once again, the client will not have
to care about conflicting MPI and Margo calls.

The situation becomes more complicated for servers. If Margo was
initialized without a dedicated ES for the progress loop, then the
progress loop will run either when the main ES calls :code:`margo_forward`,
:code:`margo_provider_forward`, or :code:`margo_wait` (just like for clients),
or when the main ES blocks on :code:`margo_wait_for_finalize`. *Everytime
the ES does anything else than these calls, the progress loop won't be running
and the server will not be able to respond to incoming RPCs*.
When using MPI in conjunction with Margo in this scenario, this means
that whenever the main ES blocks on an MPI call, it will not make progress on
Mercury operations. A common mistake we have seen users make is initializing
servers as an MPI program, initializing Margo without a dedicated progress
ES, then block on a call to :code:`MPI_Barrier` (or another MPI operation)
instead of blocking on :code:`margo_wait_for_finalize`.
The solution in such a scenario is either to use a dedicated ES for the mercury
progress loop (set the third argument of :code:`margo_init` to 1), or make sure
not to use MPI calls when the progress loop needs to be running, or to put all
MPI calls in an ES initialized separately.

If a server initializes Margo with a dedicated progress loop, there are
still cases where a conflict between MPI and Margo could occure. These cases
have to do with where the RPC handlers are executed.

Understanding *where* the RPC handlers are executed
---------------------------------------------------

In a server, when the Mercury progress loop receives an RPC request, it
will create a new ULT to run the corresponding RPC handler.
The fourth argument of :code:`margo_init` indicates *where* these ULTs
are executed. A value of -1 indicates that the RPC handlers will execute
in the same ES as the one running the progress loop. A value of 0 indicates
that they will execute in the ES that called :code:`margo_init` (the main ES).
Finally a positive value *N* will make Margo create *N* new ES where these
RPC handlers will execute.

Depending on this parameter, conflicts with MPI operations may happen.
If RPC handlers execute in the same ES as the progress loop, time spent
in an RPC handler is time not spent in the progress loop. If an RPC handler
blocks on an MPI call, it will block its entire ES and therefore prevent
the progress loop from running.

If the RPC handlers execute in the same ES as :code:`margo_init`,
and MPI call that blocks the main ES will prevent RPC handlers from running.
A frequent mistake we see users make is initializing Margo with a dedicated
progress loop (third argument of :code:`margo_init` set to 1), and RPC handlers
running in the main ES (fourth argument set to 0), but block the main ES on
an :code:`MPI_Barrier`. The solution in this case is to either use -1 or a
positive number as the fourth argument to :code:`margo_init`.

Finally if the RPC handlers execute in some *N* dedicated ES, it is still
worth keeping in mind that any blocking MPI call will prevent any ULT from
running in the blocked ES. Hence, if *N* RPC handlers are blocked on MPI
calls, no other RPC handlers will be executed. This scenario is however very
unlikely since users generally don't issue MPI calls in parallel from several
threads.

Conclusion
----------

When using MPI in conjunction with Margo, users need to keep in mind that
any blocking MPI call will prevent the calling ES from yielding control to
other ULT, which in turn may prevent either the Mercury progress loop or
some RPC handlers from running. A good practice consists of initializing
Margo with a dedicated progress loop and run RPC handlers either in the
progress loop (fourth argument of -1) or in dedicated ES (fourth argument
greater than 0), and make MPI calls only from the main ES.

When initializing Margo with :code:`margo_init_pool`, or when initializing
providers and passing dedicated Argobots pools for their RPC handlers,
users need to keep track of where the ULT placed in those pools may
execute, to know whether they may conflict with MPI operations.
