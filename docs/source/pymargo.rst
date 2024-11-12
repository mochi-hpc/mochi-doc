PyMargo (Margo for Python)
==========================

PyMargo is a python binding for Margo, with an object-oriented
interface similar to that of thallium.

This section will walk you through a series of tutorials on how to use
PyMargo. We highly recommend that you read the :ref:`Margo` tutorials first.

.. important::

   While working with Mochi in Python can be convenient, please keep
   in mind that Python has a major limitation: its global interpreter
   lock (GIL). The GIL ensures that only one thread at a time executes
   Python code, which drastically limits parallelization opportunities
   in Mochi. For instance, no more than one RPC written in Python will
   be executed at any time, even if you specified multiple execution
   streams in your Margo configuration.

   More importantly, the GIL is not Argobots-aware, which means that
   a ULT blocking on acquiring it will block the ES it runs on,
   preventing other ULTs from running. This can cause deadlocks in
   certain situations, in particular if it prevents the Mercury
   progress loop from running.

   All in all, we recommend that you restrict your use of PyMargo
   to the following scenarios.

   - You want to write a Python client interface for a Mochi service
     written in C/C++ (in this case we recommend that you look at
     how Yokan does this, for instance);
   - You want to write a Mochi service based on a Python library that
     does not have a C or C++ equivalent, and you are ready to work
     with Python's constraints.


PyMargo can be :ref:`installed using spack <Installing>`, just like any other Mochi library,
using :code:`spack install py-mochi-margo`.

.. toctree::
   :maxdepth: 1

   pymargo/01_rpc.rst
   pymargo/02_bulk.rst
   pymargo/03_provider.rst
   pymargo/04_async.rst
