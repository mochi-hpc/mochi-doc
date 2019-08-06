Properly finalizing providers
=============================

In the previous tutorial, we have seen how to implement a provider
class using Thallium. For convenience in the example, the provider
was created on the stack and the call to :code:`wait_for_finalize`
was put inside its destructor. This ensured that the main function
would block when the provider instance goes out of scope, and wait
for the engine to be finalized.

This design works fine when the program runs only one provider, but
it is flawed when we want to work with multiple providers, or when
we want to be able to destroy providers before the engine is finalized
(e.g. for services that dynamically spawn providers).

In this tutorial, we will look at another design that is more suitable
to the general case of multiple providers per process.


Client
------

The client code is the same as in the previous tutorial, though we
provide it here again for convenience. We have also added a call to
:code:`shutdown_remote_engine` from the client to shutdown the engine
on the server and trigger the engine and providers finalization.

.. literalinclude:: ../../../code/thallium/15_finalize/client.cpp
       :language: cpp

Server
------

The following code sample illustrates another version of our custom 
provider class, :code:`my_sum_provider`.

.. literalinclude:: ../../../code/thallium/15_finalize/server.cpp
       :language: cpp

First, we are making the provider's constructor private and force
users to use the :code:`create` factory method. This will ensure that
any instance of the provider is created on the heap, not on the stack.

Then, we add a bunch of :code:`tl::remote_procedure` fields to keep
the registered RPCs. We use these fields to deregister the RPCs in
the provider's destructor.

The constructor of the provider also installs a finalization callback
in the engine that will call :code:`delete` on the provider's pointer
when the engine is finalized. Because we may want to delete the provider
ourselves earlier than that, we don't forget to add a call to
:code:`pop_finalize_callback` in the destructor.

.. important::
   This design works fine if the provider pushes only one finalization callback,
   but is flawed if multiple callbacks are pushed by the provider.
   Suppose the provider pushes two callbacks :code:`f` and :code:`g`, in that order,
   and :code:`g` calls :code:`delete`. Upon finalizing, the engine will call :code:`g`
   first, which itself will call :code:`pop_finalize_callback`, which will pop :code:`f`
   out. Ultimately, :code:`f` will never be called by the engine.

The design presented in this tutorial is only an example of how to better handle
the life time of provider objects. Obviously other designs could be envisioned
(e.g. with pointers to implementation, or with smart pointers, etc.).

.. important::
   Whatever the design you chose, it is important to rememver that *all thallium
   resources (mutex, endpoints, etc.) should imperatively be destroyed before
   the engine itself finalizes*.
