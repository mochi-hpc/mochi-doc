Installing
==========

This page provides general documentation for installation of Mochi.

.. note::

    There is also a more detailed step-by-step getting started guide available
    on the :ref:`Hello Mochi page<hello-mochi-label>`. *Hello Mochi* should be
    considered a beta preview at this time, but it may be particularly helpful
    if you are uncertain about how to configure the network fabric on your
    platform.  It provides a methodical, step-by-step procedure for setting up a
    Mochi environment on a new platform.  Each step includes a demonstration
    artifact that can be used to either validate the configuration or provide
    diagnostic information for support purposes.

Installing Spack and the Mochi repository
-----------------------------------------

The recommended way to install the Mochi libraries and dependencies
is to use `Spack <https://spack.readthedocs.io/en/latest/>`_.
Spack is a package management tool designed to support multiple
versions and configurations of software on a wide variety of
platforms and environments.

First, you will need to install Spack as explained
`here <https://spack.readthedocs.io/en/latest/getting_started.html>`_.

Mercury, Argobots, Margo, and Thallium are available as builtin
packages in Spack. For all the other Mochi libraries, you will need
to  clone the following git reporitory and add it as a Spack namespace.

.. code-block:: console

   $ git clone https://github.com/mochi-hpc/mochi-spack-packages.git
   $ spack repo add mochi-spack-packages

.. important::
   The above reporitory may contain newer versions of Mercury,
   Argobots, Margo, and Thallium than what is available in Spack
   by default, so we recommend using it even if you will only
   work with these libraries.

You can then check that Spack can find Margo (for example) by typping:

.. code-block:: console

   $ spack info mochi-margo

You should see something like the following.

.. code-block:: text

   AutotoolsPackage:   mochi-margo

   Description:
       A library that provides Argobots bindings to the Mercury RPC
       implementation.

   Homepage: https://github.com/mochi-hpc/mochi-margo
   ... (more lines follow) ...

Installing the Mochi libraries
------------------------------

Installing Margo is then as simple as typping the following.

.. code-block:: console

   $ spack install mochi-margo

You will notice that Spack also installs Mercury and Argobots, since these
are needed by Margo, as well as other dependencies.

You can install Thallium using :code:`spack install mochi-thallium` (this will
install Margo if you didn't install it before, as well as its dependencies).

:code:`spack install mercury` can be used to install Mercury, and
:code:`spack install argobots` can be used to install Argobots, should you
need to install either independently of Margo or Thallium.
:code:`spack install mochi-abt-io` will install ABT-IO, and so on.
All the Mochi packages are prefixed with :code:`mochi-`.

Loading and using the Mochi libraries
-------------------------------------

It is recommended to use a Spack environment to install your Mochi packages,
as follows.

.. code-block:: console

   $ spack env create myenv
   $ spack env activate myenv
   $ spack add mochi-margo
   $ spack install

Once installed in an environment, your packages will be ready to use.

.. note::

   You can do :code:`spack repo add mochi-spack-packages` from within your
   activated environment if you don't want to polute your global installation
   of Spack.

Using the Mochi libraries with cmake
------------------------------------

Within a cmake project, Thallium, Mercury, Yokan, and Bedrock can be found using:

.. code-block:: cmake

   find_package (mercury REQUIRED)
   find_package (thallium REQUIRED)
   find_package (yokan REQUIRED)
   find_package (warabi REQUIRED)
   find_package (flock REQUIRED)
   find_package (bedrock REQUIRED)
   find_package (bedrock-module-api REQUIRED) # for compiling bedrock modules

To make cmake find Margo, Argobots, or ABT-IO, you can use
cmake's PkgConfig module:

.. code-block:: cmake

   find_package (PkgConfig REQUIRED)
   pkg_check_modules (MARGO REQUIRED IMPORTED_TARGET margo)
   pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)
   pkg_check_modules (ABTIO REQUIRED IMPORTED_TARGET abt-io)

You can now link targets as follows.

.. code-block:: cmake

   # Code using Mercury
   add_executable (my_mercury_prog source.c)
   target_link_libraries (my_mercury_prog mercury)

   # Code using Margo
   add_executable (my_margo_prog source.c)
   target_link_libraries (my_margo_prog PkgConfig::MARGO)

   # Code using Thallium
   add_executable (my_thallium_prog source.cpp)
   target_link_libraries (my_thallium_prog thallium)

   # Code using Argobots
   add_executable (my_abt_prog source.c)
   target_link_libraries (my_abt_prog PkgConfig::ABT)

   # Code using ABT-IO
   add_executable (my_abt_io_prog source.c)
   target_link_libraries (my_abt_io_prog PkgConfig::ABTIO)

   # Code using Yokan
   add_executable (my_yokan_prog source.c)
   target_link_libraries (my_yokan_prog yokan::client)
   # link against yokan::server if you need to instantiate a provider

   # Code using Warabi
   add_executable (my_warabi_prog source.cpp)
   target_link_libraries (my_warabi_prog warabi::client)
   # Note: Warabi provides the following targets to link against:
   # - warabi::client: C++ client library
   # - warabi::server: C++ server library
   # - warabi::c-client: C client library
   # - warabi::c-server: C server library

   # Code using Flock
   add_executable (my_flock_prog flock.c)
   target_link_libraries (my_flock_prog flock::client)
   # Use flock::server if you need to instantiate a provider

   # Code using Bedrock
   add_executable (my_bedrock_prog source.cpp)
   target_link_libraries (my_bedrock_prog bedrock::client)
   # Note: Bedrock is implemented in two separate packages,
   # providing a total of three libraries:
   # - bedrock::client (from mochi-bedrock package): Client library
   # - bedrock::server (from mochi-bedrock package): Server library
   # - bedrock::module-api (from mochi-bedrock-module-api): For building a module

Using the Mochi libraries with pkg-config
-----------------------------------------

While it is recommended to use CMake when working with Mochi libraries,
all the Mochi libraries can also be found using :code:`pkg-config`.
For examples:

.. code-block:: console

   $ pkg-config --libs margo
