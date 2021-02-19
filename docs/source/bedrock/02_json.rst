Bedrock's JSON format
=====================

This section details how to configure a Bedrock daemon using a JSON
file. The code bellow is an example of such a JSON configuration.

.. literalinclude:: ../../../code/bedrock/02_json/example.json
   :language: json

Margo section (optional)
------------------------

The first section that such a JSON file may contain is a :code:`margo` section.
The format for this section is explained in the Margo tutorials and
will not be covered here. Note that if not provided, default values will be
used, hence the Margo instance created by Bedrock will still rely on an Argobots
pool called :code:`__primary__`.

.. note::
   In the following sections, keep in mind that an Argobots configuration
   always has an implicite :code:`__primary__` pool, even if it does
   not appear in the configuration. This pool can always be refered to
   by name.

Bedrock section (optional)
--------------------------

The :code:`bedrock` section can provide Bedrock-specific configuration
parameters, including the Argobots pool in which Bedrock RPCs should
execute, and the provider id used by Bedrock. All the Bedrock daemons
in a given service should use the same provider id. There is usually
no reason to change this id, unless Bedrock is embedded inside an
application and multiple Bedrock instances are running alongside each other.

ABT-IO section (optional)
-------------------------

The :code:`abt_io` section describes ABT-IO instances to initialize.
It is an array of objects in the form :code:`{ "name" : "...", "pool" : "..." }`.
The name given to an instance will be used later to resolve dependencies.
The pool must refer to one of the Argobots pools defined in the :code:`margo` section
of the JSON file. The pool can be referred either by name or by index.

SSG section (optional)
----------------------

The :code:`ssg` section describes SSG groups to initialize or join.
It is an array of objects in the following form.

.. code-block:: JSON

   {
       "name" : "<string>",
       "bootstrap" : "<string>",
       "group_file" : "<string>",
       "pool" : "<string|int>",
       "credential" : "<int>",
       "swim" : {
           "period_length_ms" : "<int>",
           "suspect_timeout_periods" : "<int>",
           "subgroup_member_count" : "<int>",
           "disabled" : "<bool>"
       }
   }


- The "name" is used for other components to be able to refer to the group;
- The "bootstrap" method must be one of "init", "join", "mpi", "pmix";
- "group_file" should be the name of the file to create or use for this group;
- "pool" should refer to a pool defined in the :code:`argobots` section, either
  by name or by index;
- "credential" is a number used when the underlying network requires a
  security token to be specified;
- The "swim" section provides the parameters for the SWIM protocol run by
  this group.

Apart from the name, bootstrap, and group file, the rest of the fields are
optionals.

If the bootstrap method is "init", the process will initialize a group with
only itself as a member. If "mpi" is specified, the process will bootstrap
a group from :code:`MPI_COMM_WORLD`. If "pmix" is specified, the process
will use PMIx to bootstrap the group. In these three cases, the group file
specified will be created (or overwritten if it already exists), and the
"swim" section will be used to setup the SWIM protocol. The last
bootstrap method, "join", lets a process join an existing group. The
group file must already exist, and the "swim" parameters will be ignored
and queried from existing members of the group.

Libraries, clients,  and providers
----------------------------------

The :code:`libraries` section associates component (or "module") names with
shared libraries to load. These libraries tell Bedrock how to instantiate
a provider, a client, and provider handles of a given component type.
The next tutorial details how to write these libraries.

The :code:`clients` section is an array of client objects of the following form.

.. code-block:: JSON

   {
        "name" : "<string>",
        "type" : "<string>",
        "config" : "<object>",
        "dependencies" : {
               "<key1>" : "<name1>",
               "<key2>" : "<name2>"
        }
   }

The :code:`name` will be the name by which the client can be referred to in
other places of the configuration. The :code:`type` must be one of the module
names listed in the :code:`libraries` section.
:code:`config` should be a JSON object formatted to comply
with the component's specific JSON format. It will be passed as-is to the
component's client creation function. You should refer to the component's
documentation to know what is expected from this configuration field.

Finally the :code:`dependencies` entry is an object associating *dependency names*
to *references*. Bedrock will resolve these references and pass them to the
client creation function when calling it.

.. note::
   Bedrock can create clients as needed (e.g. when a provider depends on one or
   depends on a provider handle), so there is usually no need to fill up manually
   the :code:`clients` section of the configuration. The only reasons for doing
   so would be (1) a client needs to be passed a specifid configuration, (2)
   a client needs to be provided with required dependencies, (3) you want to
   instantiate and use multiple clients of the same type.


The :code:`providers` section is an array of provider objects of the following form.

.. code-block:: JSON

   {
        "name" : "<string>",
        "type" : "<string>",
        "provider_id" : "<int>",
        "pool" : "<string|int>",
        "config" : "<object>",
        "dependencies" : {
               "<key1>" : "<name1>",
               "<key2>" : "<name2>"
        }
   }

The :code:`name` will be the name by which the provider can be referred to in
other places of the configuration. The :code:`type` must be one of the module
names listed in the :code:`libraries` section. The :code:`provider_id` must
be an integer between 0 and 32767 (max uint16). Multiple providers of the same
type need to have distinct provider ids. The :code:`pool` must be a reference,
either by name or index, to a pool defined in the :code:`argobots` section
of the configuration. :code:`config` should be a JSON object formatted to comply
with the component's specific JSON format. It will be passed as-is to the
component's provider registration function. You should refer to the component's
documentation to know what is expected from this configuration field.

Finally the :code:`dependencies` entry is an object associating *dependency names*
to *references*. Bedrock will resolve these references and pass them to the
provider creation function when calling it.

Dependency resolution
---------------------

The :code:`dependencies` section in a provider or client lists dependency names associated with
values. These values can be one of the following.

- The name of an SSG group (for SSG dependencies) will resolve into the correponding
  SSG group handle, defined in the :code:`ssg` section of the configuration.
- The name of an ABT-IO instance (for ABT-IO dependencies) will resolve into the
  corresponding ABT-IO instance, defined in the :code:`abt_io` section of the
  configuration.
- The name of a client defined in the same JSON file;
- For providers, the name of another provider defined in the same JSON file
  (such provider must have been defined before) will resolve to a handle to this provider;
- A string of the form :code:`"<type>:client"` where *<type>* is a type of component
  will resolve into a handle for a client of the corresponding component type. If a client
  was already defined in the :code:`clients` section for the requested type, it will be
  used, otherwise Bedrock will attempt to create one. If multiple clients of the same type
  have been defined, the first one will be used;
- A string of the form :code:`"<name>@<location>"` or :code:`"<type>:<id>@<location>"`
  will resolve into a provider handle pointing to a specific provider identified
  either by its *<name>* or by its *<type>* and provider *<id>* at the specified
  *<location>*. The location may be either :code:`local`, to refer to the calling
  process, or a Mercury address (e.g. :code:`na+sm://2317/0`) pointing to a remote
  Bedrock daemon, or an SSG address in the form :code:`ssg://<group_name>/<rank>`,
  which will be resolved into the address of the specified rank in the specified
  SSG group.

The documentation for a given component should provide you with the list of
dependencies that must be provided, as well as their types. Some of these dependencies
may be listed as optional, some may be mandatory, in which case Bedrock will fail
if the dependency isn't provided in the :code:`dependencies` section of the provider.
Some dependencies may be an array, some may be a single string.

.. important::
   Clients are initialized before providers, hence they cannot depend on providers.


Working with multiple clients
-----------------------------

Suppose we have a provider :code:`ProviderA`
of type :code:`module_a`. Given the following provider handle specification:
:code:`ProviderA@ssg://mygroup/0`, Bedrock will by default use the first client
of type :code:`module_a` it finds to create the provider handle. If such a client
does not exist, Bedrock will create a default one named :code:`__module_a_client__`.

Let's now assume we have defined two clients :code:`ClientA1` and :code:`ClientA2`
in the :code:`clients` section of our JSON file, Bedrock will use :code:`ClientA1`
by default to create the provider handle. To sepcify that we want Bedrock to use
:code:`ClientA2`, we can write the provider handle specification as follows:
:code:`ClientA2->ProviderA@ssg://mygroup/0`.

.. note::
   When querying Bedrock's configuration, you will find that the :code:`clients`
   section is present regardless of whether you initially provided id, and has been
   completed with clients that Bedrock needed to create by itself. You will also
   note that all the provider handle specifications have been resolved into the
   following format: :code:`client->type:id@address` where *client* is the
   name of the client used, *type* is the type of provider, *id* is the provider id,
   and *address* is the Mercury address.
