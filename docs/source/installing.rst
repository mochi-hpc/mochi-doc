Installing
==========

The recommended way to install the Mochi libraries and dependencies 
is to use `Spack <https://spack.readthedocs.io/en/latest/>`_.
Spack is a package management tool designed to support multiple
versions and configurations of software on a wide variety of
platforms and environments.

Installing Spack and the SDS repository
---------------------------------------

First, you will need to install Spack as explained
`here <https://spack.readthedocs.io/en/latest/getting_started.html>`_.
Once Spack is installed and available in your path, clone the following
git reporitory and add it as a Spack namespace.

.. code-block:: console

   git clone https://xgitlab.cels.anl.gov/sds/sds-repo.git
   spack repo add sds-repo

You can then check that Spack can find Margo (for example) by typping:

.. code-block:: console

   spack info margo

You should see something like the following.

.. code-block:: console

   AutotoolsPackage:   margo
   
   Description:
       A library that provides Argobots bindings to the Mercury RPC
       implementation.

   Homepage: https://xgitlab.cels.anl.gov/sds/margo
   ... (more lines follow) ...

Installing the Mochi libraries
------------------------------

Installing Margo is then as simple as typping the following.

.. code-block:: console

   spack install margo

You will notice that Spack also installs Mercury and Argobots, since these
are needed by Margo, as well as other dependencies.

You can install Thallium using :code:`spack install thallium` (this will
install Margo if you didn't install it before, as well as its dependencies).

:code:`spack install mercury` can be used to install Mercury, and
:code:`spack install argobots` can be used to install Argobots, should you
need to install either independently of Margo or Thallium.

Loading and using the Mochi libraries
-------------------------------------

Once installed, you can load Margo using the following command.

.. code-block:: console

   spack load -r margo

This will load Margo and its dependencies (Mercury, Argobots, etc.).
:code:`spack load -r thallium` will load Thallium and its dependencies
(Margo, Mercury, Argobots, etc.). You are now ready to use the Mochi libraries!
