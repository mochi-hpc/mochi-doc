Writing a Bedrock module
========================

.. note::

   As of Bedrock 0.15.0, a Bedrock module can only be written in C++,
   and you will need to enable at least the C++17 standard with your compiler.

If you have programmed your own Mochi component, writing
a module to make it usable with Bedrock is really not difficult.
Such a module consists of a single dynamic library (.so) that
can be implemented as shown in the example bellow.

.. literalinclude:: ../../../code/bedrock/04_module/module.cpp
   :language: cpp

You must compile your module as a dynamic library and link it against
the :code:`bedrock::module-api` CMake target (or use :code:`pkg-config`
to lookup the flags for the :code:`bedrock-module-api` package).
If you provide your Mochi component as a Spack package, you will want
to add a dependency on :code:`mochi-bedrock-module-api`.

The following explains in more detail how such a module works. We assume
that you have a :code:`MyProvider` structure (or type definition)
representing an instance of provider for your Mochi component.

The Bedrock component presents itself in the form of a class
(:code:`MyComponent`, here) inheriting from :code:`bedrock::AbstractComponent`.
This class must be registered (in a .cpp file) with the :code:`BEDROCK_REGISTER_COMPONENT_TYPE`
macro so that Bedrock can find it upon loading the library.

Component dependencies
----------------------

The class must provide two static functions: :code:`GetDependencies` and :code:`Register`.
The :code:`GetDependencies` function will be called with an instance of
:code:`bedrock::ComponentArgs`, and should return a vector of :code:`bedrock::Dependency`.
The latter is defined as follows.

.. code-block:: cpp

   struct ComponentArgs {
       std::string              name;         // name of the component
       thallium::engine         engine;       // thallium engine
       uint16_t                 provider_id;  // provider id
       std::vector<std::string> tags;         // Tags
       std::string              config;       // JSON configuration
       ResolvedDependencyMap    dependencies; // dependencies
   };

It provides the name of the component, the thallium engine of the process,
the provider ID of the component to instantiate, a list of tags, and a configuration
string which is guaranteed to be valid JSON. The :code:`dependencies` field is empty
when :code:`GetDependencies` is called.

The purpose of the :code:`GetDependencies` function is to tell Bedrock what
dependencies your component needs, given the information (such as configuration,
tags, name, etc.) it was provided with.

The :code:`bedrock::Dependency` structure is defined as follows.

.. code-block:: cpp

   struct Dependency {
       std::string name;
       std::string type;
       bool        is_required;
       bool        is_array;
       bool        is_updatable;
   };

The name is the name by which the dependency is known in the "dependencies" section
of a component, in a Bedrock configuration. The type is the type of dependency, which
can be either "pool", "xstream", or the name of a Mochi component (e.g. "warabi", "yokan", etc.)
if the dependency should be a provider handle or a provider instance.

The is_required field indicates whether the dependency is required.
The is_array field indicates whether more than one dependency may be specified
for this dependency name.
The is_updatable field indicates whether the dependency can be updated via a call to
:code:`changeDependency`.

Given the above dependency declarations for our module, a valid provider
instantiation in the JSON document might look like the following.

.. code-block:: json

   {
        "libraries" : [
            "path/to/libbedrock-my-module.so",
            "libyokan-bedrock-module.so"
        ],
        "providers" : [
            {
                "name" : "MyProvider",
                "type" : "my_module",
                "provider_id" : 42,
                "config" : {},
                "dependencies" : {
                    "pool" : "my_pool",
                    "kv_store" : [ "yokan:33@tcp://localhost:1234", "other_db@local" ]
                }
            },
            ...
        ]
   }

The :code:`libraries` section must contain the dynamic library to load for your module.
To be valid (1) the current process has an Argobots pool called "my_pool", (2)
there should exist a Yokan provider with provider ID 33 at :code:`tcp://localhost:1234`,
and (3) there should be another Yokan provider local to the current process and named "other_db".


Component instantiation
-----------------------

Once :code:`GetDependencies` has been called and Bedrock has looked up the dependencies,
the :code:`Register` static function is called. This time, the :code:`dependencies` field
of the :code:`bedrock::ComponentArgs` has been filled. This field is of type :code:ResolvedDependencyMap`,
which is defined as a map from dependency names to a :code:`NamedDependencyList` object.
A :code:`NamedDependencyList` is itself a vector of shared pointers to a :code:`NamedDependency`,
which is defined in :code:`bedrock/NamedDependency.hpp` in the mochi-bedrock-module-api package.
Such NamedDependency wraps various types objects: Argobots pools and xstreams, as well as
Thallium provider_handles, and handles to user-defined Mochi components. The :code:`getHandle`
method may be used to extract the underlying handle of the dependency. For Argobots pools,
:code:`getHandle<thallium::pool>()` should be used. For Argobots execution streams,
:code:`getHandle<thallium::xstream>()` should be used. For Thallium provider handles,
:code:`getHandle<thallium::provider_handle>()` should be used. For direct handles to other local
components, :code:`getHandle<bedrock::ComponentPtr>()` should be used. :code:`ComponentPtr` is
defined as :code:`std::shared_ptr<AbstractComponent>`.


Component member functions
--------------------------

The component class must provide at least a :code:`getHandle()` method returning a :code:`void*`
pointer to the underlying handle of the component. It may also provide the following functions:
:code:`getConfig`, :code:`changeDependency`, :code:`migrate`, :code:`snapshot`, and :code:`restore`.
You will find more information about the expected semantics of these functions in the comments
of the :code:`bedrock/AbstractComponent.hpp` header.
