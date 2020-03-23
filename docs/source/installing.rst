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

   spack info mochi-margo

You should see something like the following.

.. code-block:: console

   AutotoolsPackage:   mochi-margo
   
   Description:
       A library that provides Argobots bindings to the Mercury RPC
       implementation.

   Homepage: https://xgitlab.cels.anl.gov/sds/margo
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

   spack load -r mochi-margo

This will load Margo and its dependencies (Mercury, Argobots, etc.).
:code:`spack load -r mochi-thallium` will load Thallium and its dependencies
(Margo, Mercury, Argobots, etc.). You are now ready to use the Mochi libraries!

Using the Mochi libraries with pkg-config
-----------------------------------------

Once loaded, all the Mochi libraries can be found using :code:`pkg-config`.
For examples:

.. code-block:: console

   $ pkg-config --libs margo

Using the Mochi libraries with cmake
------------------------------------

Within a cmake project, Thallium and Mercury can be found using:

.. code-block:: console
   
   find_package(mercury REQUIRED)
   include_directories(${MERCURY_INCLUDE_DIR})
   find_package(thallium REQUIRED)

To make cmake find Margo, Argobots, or ABT-IO, download
`this file <https://xgitlab.cels.anl.gov/sds/mochi-doc/blob/master/code/cmake/xpkg-import.cmake>`_
and place it in a *cmake* folder in your project.
In the root CMakeLists.txt file of your project, add
:code:`set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")`
and :code:`include (xpkg-import)`. You can then find Margo, Argobots, and ABT-IO using the following:

.. code-block:: console

   xpkg_import_module (argobots REQUIRED argobots)
   xpkg_import_module (margo REQUIRED margo)
   xpkg_import_module (abtio REQUIRED abt-io)
   xpkg_import_module (ssg REQUIRED ssg)

You can now link targets as follows.

.. code-block:: console
   
   # Code using Mercury
   add_executable(my_mercury_prog source.c)
   target_link_libraries(my_mercury_prog mercury)

   # Code using Margo
   add_executable(my_margo_prog source.c)
   target_link_libraries(my_margo_prog margo)

   # Code using Thallium
   add_executable(my_thallium_prog source.cpp)
   target_link_libraries(my_thallium_prog thallium)
   
   # Code using Argobots
   add_executable(my_abt_prog source.c)
   target_link_libraries(my_abt_prog abt)

   # Code using ABT-IO
   add_executable(my_abt_io_prog source.c)
   target_link_libraries(my_abt_io_prog abt-io abt)

   # Code using SSG
   add_executable(my_ssg_prog source.c)
   target_link_libraries(my_ssg_prog ssg)
