Getting started with Flock
==========================

Installing Flock
----------------

Flock can be installed using Spack as follows.

.. code-block:: console

   spack install mochi-flock +bedrock

The :code:`+bedrock` variant will enable :ref:`Bedrock` support, which will be useful
for spinning up a Flock provider without having to write code.
The :code:`spack info mochi-flock` command can be used to show the list of variants
available.

In the following sections, the code can be compiled and linked against the *flock-server*
and *flock-client* libraries, which can be found either by calling
:code:`find_package(flock)` in CMake, or :code:`pkg-config --libs --cflags flock-server`
(respectively :code:`flock-client`) with PkgConfig.

What is Flock?
--------------

Flock is a group management microservice for Mochi. It provides capabilities for:

- **Group Formation**: Creating groups of distributed processes or providers
- **Membership Discovery**: Discovering other members of a group
- **Dynamic Membership**: Supporting processes joining and leaving groups
- **Multiple Bootstrap Methods**: Initializing groups via self, view files, MPI, join, or file-based methods
- **Multiple Backends**: Static or centralized group management implementations

Flock is particularly useful for building fault-tolerant distributed services,
implementing elastic services that can scale up or down, and coordinating multiple
instances of a microservice.

Instantiating a Flock provider
-------------------------------

Flock adopts the typical Mochi microservice architecture
(used for instance in the :ref:`Margo microservice template<margo-microservice-template>`),
with a *server* library providing the microservice's *provider* implementation,
and a *client* library providing access to its capabilities (e.g., querying group membership).
A provider is what manages a group, hence the first thing we need to do is instantiate a provider.

Since we have enabled Bedrock support, let's take advantage of that and write
a *bedrock-config.json* file for Bedrock to use (if you are not familiar with Bedrock,
I highly recommend you to read the :ref:`Bedrock` section. Using Bedrock will save
you development time since it allows you to bootstrap a Mochi service using a JSON file
instead of writing code).

.. code-block:: json

   {
       "libraries": [
           "libflock-bedrock-module.so"
       ],
       "providers": [
           {
               "type": "flock",
               "name": "my_flock_provider",
               "provider_id": 42,
               "config": {
                   "bootstrap": "self",
                   "group": {
                       "type": "static",
                       "config": {}
                   },
                   "file": "mygroup.flock"
               }
           }
       ]
   }

We can now give this config file to Bedrock as follows.

.. code-block:: console

   $ bedrock na+sm -c bedrock-config.json
   [info] [flock] Flock provider started
   [info] Bedrock daemon now running at na+sm://12345-0

We now have a Flock provider running, with a *provider id* of 42, managing a group
using the "self" bootstrap method and a "static" backend. The group view will be
persisted to the file "mygroup.flock".

If you need to create a provider in C (either because you don't want to use Bedrock
or because you want your provider to be embedded into an existing application), the
following code shows how to do that.

.. literalinclude:: ../../../code/flock/01_intro/server.c
   :language: c

The key steps in this code are:

1. Initialize Margo in server mode
2. Initialize the provider arguments structure
3. Create an initial group view using :code:`flock_group_view_init_from_self` for a single-member group
4. Specify the configuration (static backend in this case)
5. Register the provider with :code:`flock_provider_register`

Interacting with the group via the client interface
---------------------------------------------------

Now we can use the client library to create a client object,
create a group handle, and interact with our group.

.. literalinclude:: ../../../code/flock/01_intro/client.c
   :language: c

The client is created using :code:`flock_client_init`.
We then create a :code:`flock_group_handle_t`. This handle
is the object that will let us interact with the group.

The group handle is created using :code:`flock_group_handle_create`, which takes:

- The client object
- The server address
- The provider ID
- A boolean indicating whether to refresh the group view automatically
- A pointer to store the resulting handle

:code:`flock_group_handle_release` should be called to destroy
the group handle. :code:`flock_client_finalize` is then used
to finalize the client.

This program can be called as follows (changing the address as needed).

.. code-block:: console

   $ ./client na+sm://12345-0 42

Bootstrap methods
-----------------

Flock supports multiple bootstrap methods for initializing groups:

- **self**: Creates a single-member group with just the current process
- **view**: Initializes from a provided group view structure
- **mpi**: Uses MPI to bootstrap a group from all MPI processes
- **join**: Joins an existing group by contacting a member
- **file**: Loads group membership from a file

The bootstrap method is specified in the configuration when creating a provider.
Later tutorials will cover each of these methods in detail.

Backend types
-------------

Flock provides two backend implementations:

- **static**: For groups with fixed membership that doesn't change after initialization
- **centralized**: For groups with dynamic membership, using a centralized coordination approach

The backend type is specified in the "group" section of the configuration.
Later tutorials will cover the differences between these backends and when to use each.

Next steps
----------

- :doc:`02_bootstrap_self`: Learn about the "self" bootstrap method in detail
- :doc:`03_bootstrap_view`: Learn about initializing from a group view
- :doc:`04_bootstrap_mpi`: Learn about MPI-based bootstrapping
