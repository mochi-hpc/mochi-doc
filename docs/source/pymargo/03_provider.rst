Providers
=========

Providers are objects that encapsulate a number of fonctions
exposed as RPCs. While the previous tutorials already show
how one can register methods of an object's instance to use
as and RPC, PyMargo provides a number of decorators to
automatize the registration of such functions.

The following server code shows these decorators.

.. literalinclude:: ../../../code/pymargo/03_provider/server.py
   :language: python

The :code:`provider` decorator can be used to decorate a class
with a service name. This service name will be prepended to
function names, along with a "_", to make up the full RPC names.

The :code:`remote()` decorator indicates that a function is
remotely callable. When registered, the RPC name used will be
the function name, unless an :code:`rpc_name` parameter is
provided to the decorator.
The decorator also takes an optional :code:`disable_response`
parameter.

The :code:`on_prefinalize` and :code:`on_finalize` decorators
are here to specify that a function should be called before
the engine is finalized, and during finalization (the difference
being that in pre-finalization phase, it is still allowed to
send and receive RPCs, while in finalization phase it is not).

Once an instance of :code:`MyProvider` is created, we simply
pass it to :code:`engine.register_provider` and the engine
will discover the functions marked as RPCs and the pre-finalization
and finalization callbacks. This function also takes an optional
:code:`provider_id` argument, so that multiple instances can be
registered, distinguished by their provider id.

The following code shows the corresponding client.

.. literalinclude:: ../../../code/pymargo/03_provider/client.py
   :language: python

The client needs to build the full RPC name by itself
(here "my_service_say_hello"). The :code:`on` function, which
binds a :code:`RemoteFunction` with an :code:`Address`, takes
an optional :code:`provider_id` argument.

.. important::
   The code above also protects the call to the RPC with a
   :code:`try ... except`. It is important in PyMargo code
   to catch exceptions and properly act on them. Uncought,
   an exception could make objects like RPC handles outlive
   the engine that created them, causing segmentation faults
   when the garbage collector tries to free them.

.. note::
   We have demonstrated here the registration of a provider
   object, but since in Python everything is an object, we
   can also pass to :code:`register_provider` a module with
   global function decorated in the same manner.
