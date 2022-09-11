Installing
==========

The recommended way to install the Mochi libraries and dependencies
is to use `Spack <https://spack.readthedocs.io/en/latest/>`_.
Spack is a package management tool designed to support multiple
versions and configurations of software on a wide variety of
platforms and environments.

Hello Mochi
-----------

.. important::
   `Hello Mochi
   <https://wordpress.cels.anl.gov/mochi/wp-content/uploads/sites/51/2022/09/hello-mochi.pdf>`_
   should be considered beta quality at this time.  Please report feedback
   to the public mailing list or Slack space (links for these can be found
   on the right column of the `Mochi web site
   <https://www.mcs.anl.gov/research/projects/mochi/>`_).

In addition to the general installation documentation provided on this
Read the Docs page, you can also find a more detailed getting
started document called `Hello Mochi <https://wordpress.cels.anl.gov/mochi/wp-content/uploads/sites/51/2022/09/hello-mochi.pdf>`_.

Hello Mochi is a methodical, step-by-step procedure for setting up a Mochi
environment on a new platform.  Each step includes a demonstration artifact
that can be used to either validate the configuration or provide diagnostic
information for support purposes.  Hello Mochi may be particularly helpful
if you are uncertain about how to configure the network fabric on your
platform.

Installing Spack and the Mochi repository
-----------------------------------------

First, you will need to install Spack as explained
`here <https://spack.readthedocs.io/en/latest/getting_started.html>`_.

Mercury, Argobots, Margo, and Thallium are available as builtin
packages in Spack. For all the other Mochi libraries, you will need
to  clone the following git reporitory and add it as a Spack namespace.

.. code-block:: console

   git clone https://github.com/mochi-hpc/mochi-spack-packages.git
   spack repo add mochi-spack-packages

.. important::
   The above reporitory may contain newer versions of Mercury,
   Argobots, Margo, and Thallium than what is available in Spack
   by default, so we recommend using it even if you will only
   work with these libraries.

You can then check that Spack can find Margo (for example) by typping:

.. code-block:: console

   spack info mochi-margo

You should see something like the following.

.. code-block:: console

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

   spack install mochi-margo

You will notice that Spack also installs Mercury and Argobots, since these
are needed by Margo, as well as other dependencies.

You can install Thallium using :code:`spack install mochi-thallium` (this will
install Margo if you didn't install it before, as well as its dependencies).

:code:`spack install mercury` can be used to install Mercury, and
:code:`spack install argobots` can be used to install Argobots, should you
need to install either independently of Margo or Thallium.
:code:`spack install mochi-abt-io` will install ABT-IO.
:code:`spack install mochi-ssg` will install SSG.

Loading and using the Mochi libraries
-------------------------------------

Once installed, you can load Margo using the following command.

.. code-block:: console

   spack load mochi-margo

This will load Margo and its dependencies (Mercury, Argobots, etc.).
:code:`spack load mochi-thallium` will load Thallium and its dependencies
(Margo, Mercury, Argobots, etc.). You are now ready to use the Mochi libraries!

Using the Mochi libraries with pkg-config
-----------------------------------------

Once loaded, all the Mochi libraries can be found using :code:`pkg-config`.
For examples:

.. code-block:: console

   $ pkg-config --libs margo

Using the Mochi libraries with cmake
------------------------------------

Within a cmake project, Thallium, Mercury, Yokan, and Bedrock can be found using:

.. code-block:: console

   find_package(mercury REQUIRED)
   find_package(thallium REQUIRED)
   find_package(yokan REQUIRED)
   find_package(bedrock REQUIRED)

To make cmake find Margo, Argobots, ABT-IO, or SSG, you can use
cmake's PkgConfig module:

.. code-block:: console

   find_package (PkgConfig REQUIRED)
   pkg_check_modules (MARGO REQUIRED IMPORTED_TARGET margo)
   pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)
   pkg_check_modules (ABTIO REQUIRED IMPORTED_TARGET abt-io)
   pkg_check_modules (SSG REQUIRED IMPORTED_TARGET ssg)

You can now link targets as follows.

.. code-block:: console

   # Code using Mercury
   add_executable(my_mercury_prog source.c)
   target_link_libraries(my_mercury_prog mercury)

   # Code using Margo
   add_executable(my_margo_prog source.c)
   target_link_libraries(my_margo_prog PkgConfig::MARGO)

   # Code using Thallium
   add_executable(my_thallium_prog source.cpp)
   target_link_libraries(my_thallium_prog thallium)

   # Code using Argobots
   add_executable(my_abt_prog source.c)
   target_link_libraries(my_abt_prog PkgConfig::ABT)

   # Code using ABT-IO
   add_executable(my_abt_io_prog source.c)
   target_link_libraries(my_abt_io_prog PkgConfig::ABTIO)

   # Code using SSG
   add_executable(my_ssg_prog source.c)
   target_link_libraries(my_ssg_prog PkgConfig::SSG)

   # Code using Bedrock
   add_executable(my_bedrock_prog source.cpp)
   target_link_libraries(my_bedrock_prog bedrock-client)
   # link against bedrock-server if you need an embedded server

   # Code using Yokan
   add_executable(my_yokan_prog source.cpp)
   target_link_libraries(my_yokan_prog yokan-client yokan-server yokan-admin)
   # select the relevant library to link against
