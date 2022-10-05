RPCs and responses
==================

In this tutorial, we will see how to initialize PyMargo,
register an RPC function, and call it from a client.

PyMargo server
--------------

The following code initializes a PyMargo Engine for use as a server,
then prints the address at which the server can be contacted.
It exposes a "hello" RPC that takes a firstname and a lastname,
prints them, and returns a string.

.. literalinclude:: ../../../code/pymargo/01_rpc/server.py
   :language: python

The :code:`Engine`'s constructor takes the protocol as the first
argument. Additional arguments may be provided:

- :code:`mode`: :code:`pymargo.client` or :code:`pymargo.server`,
  to indicate whether the engine will be used as a client or a
  server (defaults to server);
- :code:`use_progress_thread`: a boolean indicating whether to
  use a progress thread or not;
- :code:`num_rpc_threads`: the number of Argobots ES for servicing
  RPCs. Note that because of the GIL, using a value other than 0
  will not necessarily result in better concurrency;
- :code:`config`: a dictionary or a JSON-formatted string representing
  a Margo configuration.

We then call :code:`engine.register` to associate an RPC name with
a function. Since in Python everything is an object, you can pass
any callable to the :code:`engine.register` function, including
methods bound to instances, or instances of objects with a :code:`__call__`
method.

The :code:`register` function also accepts the following optional
arguments:

- :code:`provider_id`: an integer equal or greater than 0 and less than 65535.
  This argument is used when working with providers.
- :code:`disable_response`: a boolean indicating that the RPC will not send
  a response back.

The :code:`engine.address` call inside our print statement returns an
:code:`Address` object corresponding to the address of our server.
This address is here converted into a string when printed.

The call to :code:`enable_remote_shutdown` on the engine will allow a remote
process to ask our server to shutdown.

Looking at the :code:`hello` function, the first argument
must be a :code:`Handle` object. The rest of the arguments and their numbers
can be anything, as long as their types are
`pickleable <https://docs.python.org/3/library/pickle.html>`_.
The :code:`Handle` object is used to call :code:`respond`, among other things.
:code:`handle.address` can be used to get the address of the sender.
Any object can be passed to :code:`respond`, again as long as its type
is pickleable. Here, our function takes two argument (firstname and lastname)
of expected type :code:`str`, and will respond with a string.

Note that the :code:`Engine` is used as a context manager (:code:`with` statement),
this context will automatically call :code:`finalize` on the engine
when exiting, hence we have to explicitely block on a :code:`wait_for_finalize`
call to avoid exiting the context and start servicing RPCs.

PyMargo client
--------------

The following code is the corresponding client.

.. literalinclude:: ../../../code/pymargo/01_rpc/client.py
   :language: python

In this code, we initialize the :code:`Engine` with :code:`mode=pymargo.client`.
We then use the engine's :code:`lookup` function to convert a string address
(taken from :code:`sys.argv[1]`) into an :code:`Address` object.

We call :code:`register` with only the name of the RPC.
This function returns a :code:`RemoteFunction` object.
By calling :code:`on(Address)` on this object, we get a :code:`CallableRemoteFunction`
with a :code:`__call__` operator that we can call. The code shows how
arguments (positional and keywords) will be sent to the server. Here :code:`firstname`
is provided as positional, while :code:`lastname` is provided as a keyword argument.
The returned value is the object passed to :code:`handle.respond` in the server.

Finally, :code:`address.shutdown()` sends a specific message to our server
asking it to shutdown.

.. important::
   Do not use the argument names :code:`blocking` and :code:`timeout`
   when defining RPCs (or if you do, make sure you use them as positional
   arguments when invoking the RPC). These argument keys are used by
   PyMargo to control the behavior of the call (more on that later).

.. note::
   Python installs a signal handler to catch keyboard interrupts, but
   this signal handler cannot run when the server is blocked on
   :code:`wait_for_finalize` or on C/C++ code more generally.
   Hence Ctrl-C won't kill your server.
   Due to the interactions between the GIL and Argobots, it is also
   possible that a program does not terminate after an exception is
   raised. To kill such a program, you will have to resort to :code:`kill -9`.
