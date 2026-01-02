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

Libraries and components
------------------------

The :code:`libraries` section lists shared libraries to load.
These libraries tell Bedrock how to instantiate providers for various component types.
The next tutorial details how to write these component libraries, called "modules".

The :code:`providers` section is an array of provider objects of the following form.

.. code-block:: JSON

   {
        "name" : "<string>",
        "type" : "<string>",
        "provider_id" : "<int>",
        "config" : "<object>",
        "dependencies" : {
               "<key1>" : "<name1>",
               "<key2>" : "<name2>"
        }
   }

The :code:`name` will be the name by which the provider can be referred to in
other places of the configuration. The :code:`type` must be one of the module
names provided in one of the loaded libraries. The :code:`provider_id` must
be an integer between 0 and 65534 (max uint16, minus one as 65535 is reserved).
All the providers must have distinct provider ids.
:code:`config` should be a JSON object formatted to comply
with the component's expected JSON format. It will be passed as-is (as a string)
to the component's registration function. You should refer to the component's
documentation to know what is expected from this configuration field.

Finally the :code:`dependencies` entry is an object associating *dependency names*
to *references*. Bedrock will resolve these references and pass them to the
provider creation function when calling it.

Dependency resolution
---------------------

The :code:`dependencies` section in a provider lists dependency names associated with
values. These values can be one of the following:

- **Pool**: The name of an Argobots pool listed in the :code:`"margo"` section;
- **Xstream**: The name of an Argobots execution stream listed in the :code:`"margo"` section;
- **Local provider**: The name of another provider defined in the same JSON file
  (such provider must have been defined before) will resolve to a pointer to this provider;
- **Remote provider**: A string of the form :code:`"<name>@<location>"` or :code:`"<type>:<id>@<location>"`
  will resolve into a provider handle pointing to a specific provider identified
  either by its *<name>* or by its *<type>* and provider *<id>* at the specified
  *<location>*. The location may be either :code:`local`, to refer to the calling
  process, or a Mercury address (e.g. :code:`na+sm://2317/0`) pointing to a remote
  Bedrock daemon.

The documentation for a given component should provide you with the list of
dependencies that must be provided, as well as their types. Some of these dependencies
may be listed as optional, some may be mandatory, in which case Bedrock will fail
if the dependency isn't provided in the :code:`dependencies` section of the provider.
Some dependencies may be an array, some may be a single string.

.. note::
   The :code:`clients` section from earlier versions of Bedrock has been deprecated.
   Clients should now be initialized directly using the component's client library API.
