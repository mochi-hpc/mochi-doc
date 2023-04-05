Getting started with Bake
=========================

Installing Yokan
----------------

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
