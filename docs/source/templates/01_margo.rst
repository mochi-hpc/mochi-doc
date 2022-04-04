.. _margo-microservice-template:

Margo microservice template (C)
===============================

The Margo microservice template is available
`here <https://github.com/mochi-hpc/margo-microservice-template>`_.
Though this project provides many examples of how to use the Margo API, you may
want to refer to the Margo documentation for more detail.

The Mochi philosophy
--------------------

The philosophy of the Mochi project consists of providing a set of building blocks
for developing HPC data service. Each building block is meant to offer **efficient**,
**location-agnostic** access to a simple set of functionalities through
**modular backends**, while **seamlessly sharing hardware** (compute and network)
with other building blocks.

A **simple set of functionalities** may be, for example, *"storing and retrieving
small key/value pairs"*, a common feature found in many storage systems to manage
metadata. The **location-agnostic** aspect aims at making such a feature available
to user programs in the same manner and through the same API regardless of whether
the service runs in the same process, on the same node but on different processes,
or on a different node across a network. The **modular backends** aspect makes it
possible to abstract the implementation of such a feature and, if not providing
multiple implementations, at least providing the means for someone to easily swap
the default implementation of the feature for their own. Because multiple building
blocks may be running on the same node, such a building block should efficiently
share the compute and network hardware with other building blocks. This is done
by sharing a common networking and threading layer: Margo.


Design overview
---------------

The typical design of a Mochi microservice revolves around four libraries:
*server*, *client*, *admin*, and *bedrock module*.

- The server library contains a service **provider**, that is, an object that
  can receive some predefined RPCs to offer a particular functionality. Within
  the same process, multiple providers of the same service may be instantiated,
  using distinct **provider ids** (*uint16_t*). A provider is responsible for
  managing a set of **resources**. In the example of a storage for key/value
  pairs, a resource may be a database. The functionalities of a provider may
  be enabled by multiple **backends**. For example, a database may be implemented
  using LevelDB, BerkeleyDB, or simply using an in-memory hash table.
  Programs sending requests to a provider should **not** be aware of the backend used
  to implement the requested functionality. This allows multiple backends to be
  tested, and for backends to evolve independently from user applications.
- The **client** library is the library through which user applications or higher-level
  services interact with providers. It will typically provide a client structure
  that is used to register the set of RPCs that can be invoked, and a **resource handle**
  structure that references a particular resource located on a particular provider.
  User applications will typically initialize a singe client object for a service, and
  from this client object instantiate as many resource handles as needed to interact with
  available resources. Resources are identified by a **resource id**, which are generally
  either a name, an integer, or a `uuid <https://en.wikipedia.org/wiki/Universally_unique_identifier>`_
  (this template project uses uuids).
- The **admin** library is the library through which a user application can send
  requests that are meant for the provider itself rather than for a resource.
  A few most common such requests include the creation and destruction of
  resources, their migration, etc. It can be useful to think of the admin
  library as the set of features you would want to provide to the person or
  application that sets up the service, rather than the person or application
  that uses its functionalities.
- The **bedrock module** library enables using your component with Bedrock.
  It is implemented in *src/bedrock-module.c*.

Organization of the template project
------------------------------------

The template project illustrates how a Margo-based microservice could
be architected. It can be compiled as-is, and provides a couple of
functionalities that make the provider print a "Hello World" message
on its standard output, or compute the sum of two integers.

This template project uses *alpha* as the name of your microservice.
Functions, types, files, and libraries therefore use the *alpha* prefix.
The first step in setting up this project for your microservice will be
to replace this prefix. The generic name *resource* should also be
replaced with a more specific name, such as *database*. This renaming
step is done automatically when using the template on github (see the next section).
You don't have to rename everything by yourself!

The *include* directory of this template project provides public header files.

- *alpha/alpha-common.h* contains APIs that are common to the three
  libraries, such as error codes or common types;
- *alpha/alpha-client.h* contains the client-side functions to create
  and destroy a client object;
- *alpha/alpha-resource.h* contains the client-side functions to create
  and destroy resource handles, and to interact with a resource through
  a resource handle;
- *alpha/alpha-server.h* contains functions to register and destroy
  a provider;
- *alpha/alpha-backend.h* contains the definition of a structure that
  one would need to implement in order to provide a new backend for
  your microservice;
- *alpha/alpha-admin.h* contains the functions to create and destroy
  an admin object, as well as admin functions to interact with a provider;
- *alpha/alpha-provider-handle.h* contains the definition of a provider handle.
  This type of construct is often used in Mochi services to encapsulate
  an address and a provider id.

The implementation of all these functions is located in the *src* directory.
The source also includes functionalities such as a small header-based logging library.
The *src/dummy* directory provides a default implementation of a backend. This
backend also exemplifies the use of the `json-c <https://github.com/json-c/json-c>`_ library
for JSON-based resource configuration. We recommend that you implement a dummy backend for your
service, as a way of testing application logic and RPCs without the burden of complex
external dependencies. For instance, a dummy backend may be a backend that simply
acknowledges requests but does not process them, or provides mock results.

The *examples* directory contains an example using the microservice:
the server example will start a provider and print its address (if logging was enabled).
The admin example will connect to this provider and have it create a resource, then
print the resource id. The client example can be run next to interact with the resource.

The *tests* directory contains a set of unit tests for your service.
It relies on `µnit <https://nemequ.github.io/munit>`_ (included in the repository),
a C unit-test library under an MIT license. Feel free to continue using it as you
add more functionalities to your microservice; unit-testing is just good software
development practice in general.

The template also contains a *spack.yaml* file at its root that can be used to
install its dependencies. You may add additional dependencies into this file as
your microservice gets more complex.

As you modify this project to implement your own microservice, feel free to remove
any dependencies you don't like (such as json-c or µnit) and adapt it to your needs!

Setting up your project
-----------------------

Let's assume you want to create a microservice called "yellow", which manages
a phone directory (association between names and phone numbers). The following
shows how to setup your project:

First, go to `template repository on github <https://github.com/mochi-hpc/margo-microservice-template>`_
and click "Use this template". Give it the name "yellow", and proceed.
In your new repository, edit the *initial-setup.json* file to rename your service
and your resources. IMPORTANT: these names must be lower-case and without spaces,
since they will be used in C code for identifiers, function names, etc.

Editing *initial-setup.json* will trigger a github action. Wait a couple of minutes
and you should see a new commit appear: github has renamed your files, functions, etc. by itself!
It has also removed the COPYRIGHT file and the initial-setup.json file.

Your repo is now ready to use!

Building the project
--------------------

The project's dependencies may be build using `spack <https://spack.readthedocs.io/en/latest/>`_.
You will need to have setup `mochi-spack-packages <https://github.com/mochi-hpc/mochi-spack-packages>`_ as external
namespace for spack, which can be done as follows.

.. code-block:: console

   # from outside of your project directory
   git clone https://github.com/mochi-hpc/mochi-spack-packages.git
   spack repo add mochi-spack-packages

The easiest way to setup the dependencies for this project is to create a spack environment
using the *spack.yaml* file located at the root of the project, as follows.

.. code-block:: console

   # create an anonymous environment
   cd my_project
   spack env activate .
   spack install

or as follows.

.. code-block:: console

   # create an environment named myenv
   cd my_project
   spack env create myenv spack.yaml
   spack env activate myenv
   spack install

Once the dependencies have been installed, you may build the project as follows.

.. code-block:: console

   mkdir build
   cd build
   cmake .. -DENABLE_TESTS=ON -DENABLE_EXAMPLES=ON -DENABLE_BEDROCK=ON
   make

You can test the project using :code:`make test` from the build directory.
