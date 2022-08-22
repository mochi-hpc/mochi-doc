Debugging a Mochi application
=============================

Figuring out why a particular error happens down in Mercury
or why a Mochi service is performing poorly can be difficult.
In this tutorial we will see what users can do to help diagnose
problems with Mochi codes.

First steps: Trouble Initializing
_________________________________

Mercury can use the Libfabric (OFI) transport layer for inter-node RPC
messaging.  Sometimes configuring libfabric can be tricky.
The most common issue people are seeing when starting with
Mochi is Margo failing to initialize. 99% of the time this
is due to libfabric not being compiled with the right providers.
For instance if you need :code:`tcp`, libfabric will need to be
compiler with `fabrics=tcp,rxm`.

If misconfigured, Margo and Mercury will try to help you out by reporting an
error like this:

.. code-block:: console
   :linenos:

   # [1035629.626564] mercury->fatal: [error] [..]/src/na/na_ofi.c:1807
    # na_ofi_provider_check(): Requested OFI provider "verbs;ofi_rxm" (derived from "verbs"
      protocol) is not available. Please re-compile libfabric with support for
      "verbs;ofi_rxm" or use one of the following available providers:
      tcp;ofi_rxm udp tcp sockets
   # [1035629.626629] mercury->fatal: [error] [..]/src/na/na.c:327
    # NA_Initialize_opt(): No suitable plugin found that matches verbs
   [error] Could not initialize hg_class



Note in line 2 the "Requested OFI provider" -- that is the protocol Mercury tried
to use.  In line 5, Mercury reports the providers supported by this build of
libfabric.

On some systems, only the "compute nodes" have e.g. infiniband cards.  You can verify with the :code:`fi_info -l` command.


Enabling logging in Mercury
---------------------------

More information can be obtained from Mercury by making sure
to build it with the :code:`+debug` variant. Once this is done,
setting the :code:`HG_LOG_LEVEL` to :code:`error`, :code:`warning`,
or :code:`debug`, may provide more information about what Mercury
is trying to do and why it failed.
More information on logging with Mercury can be found
`here <https://github.com/mercury-hpc/mercury/#faq>`_.

Enabling logging in libfabric
-----------------------------

If you are using a libfabric provider, logging can be enabled
in libfabric also by building it with its :code:`+debug` variant,
and by setting the :code:`FI_LOG_LEVEL` environment variable
to :code:`Warn`, :code:`Trace`, :code:`Info`, or :code:`Debug`.
More information on logging with libfabric can be found
`here <https://ofiwg.github.io/libfabric/main/man/fabric.7.html>`_.
