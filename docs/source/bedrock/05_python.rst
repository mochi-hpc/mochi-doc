Python bindings
===============

Bedrock provides Python bindings that allow you to start Bedrock services,
connect to them, query their configuration, and manipulate them at runtime,
all from Python.

Installing Bedrock with Python support
---------------------------------------

To use Bedrock's Python bindings, you need to install Bedrock with Python support
enabled using Spack:

.. code-block:: console

   spack install mochi-bedrock+python

This will build Bedrock with Python bindings and install the :code:`mochi.bedrock`
Python package.

Starting a Bedrock service from Python
---------------------------------------

The :code:`mochi.bedrock` module provides a :code:`Server` class that can be used
to start a Bedrock service directly from Python.

.. literalinclude:: ../../../code/bedrock/05_python/server.py
   :language: python

The :code:`Server` constructor takes a Mercury address (protocol) and an optional
configuration. The configuration can be provided as:

- A Python dictionary
- A JSON string
- A :code:`ProcSpec` object (see below)
- A file path (if using :code:`Server.from_config_file()`)

The server runs in its own thread and can be finalized using :code:`server.finalize()`.

Connecting to a Bedrock service
--------------------------------

The :code:`Client` class allows you to connect to a running Bedrock service
and interact with it.

.. literalinclude:: ../../../code/bedrock/05_python/client.py
   :language: python

The :code:`ServiceHandle` object returned by :code:`make_service_handle()` provides
methods to query and manipulate the service's configuration at runtime.

Querying configuration
----------------------

You can retrieve the complete configuration of a Bedrock service:

.. literalinclude:: ../../../code/bedrock/05_python/query.py
   :language: python

The :code:`config` property returns a Python dictionary representing the
current configuration of the service. You can also use Jx9 scripts to
query specific parts of the configuration.

Runtime configuration manipulation
-----------------------------------

The :code:`ServiceHandle` class provides methods to modify a running service's
configuration dynamically.

Loading modules at runtime
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Load a new module
   service.load_module("/path/to/libmy-module.so")

Adding pools and execution streams
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Add a new Argobots pool
   pool_config = {
       "name": "new_pool",
       "kind": "fifo_wait",
       "access": "mpmc"
   }
   service.add_pool(pool_config)

   # Add a new execution stream
   xstream_config = {
       "name": "new_xstream",
       "scheduler": {
           "type": "basic_wait",
           "pools": ["new_pool"]
       }
   }
   service.add_xstream(xstream_config)

   # Remove a pool (must not be in use)
   service.remove_pool("new_pool")

Adding providers at runtime
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # Add a new provider
   provider_config = {
       "name": "my_new_provider",
       "type": "yokan",
       "provider_id": 99,
       "pool": "__primary__",
       "config": {
           "database": {
               "type": "map"
           }
       }
   }
   provider_id = service.add_provider(provider_config)
   print(f"Created provider with ID: {provider_id}")

Complete example
^^^^^^^^^^^^^^^^

Here's a complete example showing runtime manipulation:

.. literalinclude:: ../../../code/bedrock/05_python/runtime.py
   :language: python

Working with service groups
----------------------------

When multiple Bedrock services are organized in a Flock group, you can
interact with them collectively using :code:`ServiceGroupHandle`:

.. literalinclude:: ../../../code/bedrock/05_python/group.py
   :language: python

The :code:`ServiceGroupHandle` allows you to:

- Query all members of the group
- Access individual service handles by index
- Refresh the group membership
- Broadcast operations to all members

Using ProcSpec for configuration
---------------------------------

The :code:`mochi.bedrock.spec` module provides Python classes for building
configurations programmatically:

.. code-block:: python

   from mochi.bedrock.spec import ProcSpec, PoolSpec, XstreamSpec, ProviderSpec

   # Create pool specifications
   pool1 = PoolSpec(name="pool1", kind="fifo_wait", access="mpmc")
   pool2 = PoolSpec(name="pool2", kind="fifo_wait", access="mpmc")

   # Create xstream specifications
   xstream1 = XstreamSpec(
       name="xstream1",
       scheduler={"type": "basic_wait", "pools": ["pool1"]}
   )

   # Create provider specification
   provider = ProviderSpec(
       name="my_provider",
       type="yokan",
       provider_id=42,
       pool="pool1",
       config={"database": {"type": "map"}}
   )

   # Build complete process specification
   spec = ProcSpec()
   spec.margo.argobots.pools = [pool1, pool2]
   spec.margo.argobots.xstreams = [xstream1]
   spec.providers = [provider]

   # Convert to dictionary or JSON
   config_dict = spec.to_dict()
   config_json = spec.to_json()

   # Start server with this configuration
   server = Server("na+sm", config=spec)

Error handling
--------------

The Python bindings raise exceptions when errors occur:

.. code-block:: python

   from mochi.bedrock import ClientException, BedrockException

   try:
       service = client.make_service_handle(address, 0)
       config = service.config
   except ClientException as e:
       print(f"Client error: {e}")
   except BedrockException as e:
       print(f"Bedrock error: {e}")

Integration with PyMargo
-------------------------

The Bedrock Python bindings integrate with PyMargo. You can extract the
underlying Margo instance:

.. code-block:: python

   import pymargo
   from mochi.bedrock import Server

   # Start Bedrock server
   server = Server("na+sm", config={})

   # Access the underlying margo instance (if needed)
   # Note: typically you don't need to access this directly
   # as Bedrock manages the Margo instance for you

Best practices
--------------

1. **Always finalize**: Call :code:`server.finalize()` or :code:`client.finalize()`
   when done to properly clean up resources.

2. **Use context managers**: Consider wrapping server/client usage in context managers
   for automatic cleanup.

3. **Check before removing**: Before removing pools or xstreams, ensure they're not
   in use by any providers.

4. **Validate configurations**: Use :code:`ProcSpec` classes to build configurations
   programmatically to avoid JSON syntax errors.

5. **Handle exceptions**: Always wrap Bedrock operations in try-except blocks to
   handle potential errors gracefully.

Next steps
----------

- :doc:`06_jx9`: Learn about Jx9 queries for advanced configuration introspection
- :doc:`07_runtime_config`: Learn about runtime configuration in C++
- :doc:`08_flock_integration`: Learn how to use Flock for group management
