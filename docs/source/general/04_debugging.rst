Debugging a Mochi application
=============================

Figuring out why a particular error happens down in Mercury
or why a Mochi service is performing poorly can be difficult.
In this tutorial we will see what users can do to help diagnose
problems with Mochi codes.

Enabling logging in Mercury
---------------------------

More information can be obtained from Mercury by making sure
to build it with the :code:`+debug` variant. Once this is done,
setting the :code:`HG_LOG_LEVEL` to :code:`error`, :code:`warning`,
or :code:`debug`, may provide more information about what Mercury
is trying to do and why it failed.
More information on logging with Mercury can be found
`here <https://github.com/mercury-hpc/mercury/#faq>`_.

.. important::
   The most common issue people are seeing when starting with
   Mochi is Margo failing to initialize. 99% of the time this
   is due to libfabric not being compiled with the right providers.
   For instance if you need :code:`tcp`, libfabric will need to be
   compiler with `fabrics=tcp,rxm`.

Enabling logging in libfabric
-----------------------------

If you are using a libfabric provider, logging can be enabled
in libfabric also by building it with its :code:`+debug` variant,
and by setting the :code:`FI_LOG_LEVEL` environment variable
to :code:`Warn`, :code:`Trace`, :code:`Info`, or :code:`Debug`.
More information on logging with libfabric can be found
`here <https://ofiwg.github.io/libfabric/main/man/fabric.7.html>`_.
