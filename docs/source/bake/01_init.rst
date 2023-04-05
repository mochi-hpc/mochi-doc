Getting started with Bake
=========================

Installing Bake
---------------

Bake can be installed using Spack as follows.

.. code-block:: console

   spack install mochi-bake +bedrock

The :code:`+bedrock` variant will enable :ref:`Bedrock` support, which will be useful
for spinning up a Bake server without having to write code.
The :code:`spack info mochi-bake` command can be used to show the list of variants available.

In the following sections, the code can be compiled and linked against the *bake-server*
and *bake-client* libraries, which can be found either by calling
:code:`pkg-config --libs --cflags bake-server` and :code:`pkg-config --libs --cflags bake-client`
with PkgConfig. Bake does not currently provide an admin library.

Starting a Bake provider and creating a target
----------------------------------------------

The following code shows how to use the Bake server library to register
a Bake provider.

.. literalinclude:: ../../../code/bake/01_init/server.c
   :language: c

The :code:`bake_provider_register` function is used to register a Bake provider.

The :code:`bake_provider_create_target` function is used to create a storage target.
The path to the target is provided, prefixed with either :code:`pmem:` or :code:`file:`.
The former uses the `PMEM <https://pmem.io/>`_ library to manage data in a fixed-size
file on a persistent memory device. The latter uses a set of files, accessed using
classical POSIX functions, to store data.

Targets are uniquely identified using :code:`bake_target_id_t`, which contains a :code:`uuid`.
The :code:`bake_target_id_to_string` function can be used to print an ASCII version of
this identifier for later use.

Starting a Bake provider via Bedrock
------------------------------------

If Bake has been built with Bedrock support, Bedrock can spin up a Bake provider using
the following JSON configuration.

.. literalinclude:: ../../../code/bake/01_init/bedrock.json
   :language: json

The path to the target is specified in the :code:`pmem_backend`
(or :code:`file_backend`) array.

The current configuration code does not allow creating a target.
The target must have been created beforehand at the specified path.
This can be done using the :code:`bake-mkpool` utility, which Bake provides:

.. code-block:: console

   bake-mkpool -s 8M "pmem:/dev/shm/mytarget.dat"


