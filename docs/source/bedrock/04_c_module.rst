Writing a Bedrock module in C
=============================

If you have programmed your own Mochi component, writing
a module to make it usable with Bedrock is really not difficult.
Such a module consists of a single dynamic library (.so) that
can be implemented as show in the example bellow.

.. literalinclude:: ../../../code/bedrock/04_c_module/module.c
   :language: c

Module dependencies
-------------------

The first thing to declare in this module is the dependencies.
This is the list of dependencies that your component's providers
are expecting. It is expressed as an array of :code:`bedrock_dependency`
structures terminated by a :code:`BEDROCK_NO_MORE_DEPENDENCIES` entry.
Each of the elements in this array has a dependency name,
a type, and a flag. The name is what will identify the dependency
in the JSON configuration file. For instance, our module here has
two dependencies, *io* and *sdskv_ph*, hence the :code:`dependencies`
field of a *module_a* provider in a JSON configuration should have
a :code:`io` entry and an :code:`sdskv_ph` entry.

The *type* of dependency tells Bedrock what to look for. The :code:`abt_io`
type tells Bedrock to find an ABT-IO instance. The :code:`sdskv` type
tells Bedrock to find an object (provider, client, or provider handle)
from the SDSKV module.

Finally the *flag* may be 0 for optional dependencies, :code:`BEDROCK_REQUIRED`
for a required dependencies, :code:`BEDROCK_ARRAY` for an array of dependencies
(including an empty array), or :code:`BEDROCK_REQUIRED | BEDROCK_ARRAY` for
an array of at least one element.

Given the above dependency declarations for our module, a valid provider
instantiation in the JSON document might look like the following.

.. code-block:: JSON

   {
        "libraries" : {
            "module_a" : "path/to/libbedrock-module-a.so"
        },
        "providers" : [
            {
                "name" : "ProviderA",
                "type" : "module_a",
                "provider_id" : 42,
                "pool" : "my_pool",
                "config" : {},
                "dependencies" : {
                    "io" : "my_abt_io",
                    "sdskv_ph" : [ "my_sdskv@ssg://my_group/0", "other_sdskv@local" ]
                }
            }
        ]
   }

The :code:`libraries` section must associate the *module_a* type with the
library we just built. Assuming an ABT-IO instance named "my_abt_io" was
declared in the :code:`abt_io` section of the document, the "io" dependency
will be resolved to that instance. The "sdskv_ph" dependency will resolve
to an array of two provider handles pointing to SDSKV providers.

Callback functions
------------------

The rest of the module consists of callback functions to register and
deregister a provider, get a provider's configuration, initialize and finalize a client,
and create and destroy a provider handle for your module.

The provider registration callback is being passed a :code:`bedrock_args_t` handle
from which we can extrat various configuration parameters, including the Margo
instance, the provider id, the Argobots pool, the configuration (:code:`config` field
from the JSON configuration of the provider), the name of the provider, as well
as its dependencies using :code:`bedrock_args_get_num_dependencies` and
:code:`bedrock_args_get_dependency` (dependencies that are not an array are treated
like an array of size 1. :code:`bedrock_args_get_num_dependencies` can also be
used to check for the presence of an optional dependency).

Note that Bedrock checks ahead of time that all required dependencies are
present, hence the provider registration function does not have to check this.

.. note::
   The function that returns a provider's configuration must return a heap-allocated
   string, that Bedrock will later :code:`free` by itself.
