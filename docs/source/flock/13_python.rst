Python API
===========

This tutorial covers the Flock Python bindings, which provide a Pythonic interface
to Flock's group management capabilities. The Python API is built on top of the
C++ implementation and integrates seamlessly with pymargo.

Prerequisites
-------------

- Basic understanding of Flock concepts (see Tutorial 01)
- Python 3.6 or later
- pymargo Python bindings
- mochi.flock Python package

Installation
------------

Install Flock with Python bindings using Spack:

.. code-block:: bash

   spack install mochi-flock +python

Or if you're building from source:

.. code-block:: bash

   cd python
   pip install .

What You'll Learn
-----------------

- Setting up Flock providers in Python
- Creating and using Flock clients
- Working with GroupView objects
- File-based bootstrapping
- Serialization and deserialization
- Different backend configurations

Quickstart Example
------------------

Here's a simple example showing the basics of Flock in Python:

.. literalinclude:: ../../../code/flock/13_python/quickstart.py
   :language: python
   :linenos:

Key Points
~~~~~~~~~~

**Imports** (lines 5-8):
  The main modules are:
  - ``mochi.flock.server`` - Provider (server-side)
  - ``mochi.flock.client`` - Client and GroupHandle
  - ``mochi.flock.view`` - GroupView class
  - ``pymargo.core`` - Margo engine

**Creating a Provider** (lines 19-25):
  .. code-block:: python

     provider = server.Provider(
         engine=engine,
         provider_id=42,
         config=json.dumps(config),
         initial_view=initial_view
     )

  The provider manages group membership. Configuration is passed as a JSON string.

**Creating a Client** (line 30):
  .. code-block:: python

     flock_client = client.Client(engine)

  Clients can connect to Flock providers to access group information.

**Creating a Group Handle** (lines 33-37):
  .. code-block:: python

     group = flock_client.make_group_handle(
         address=str(engine.address),
         provider_id=42
     )

  A GroupHandle provides access to the group view.

Server-Side: Provider API
--------------------------

**Provider Class**:
  .. code-block:: python

     from mochi.flock.server import Provider

     provider = Provider(
         engine: pymargo.core.Engine,
         provider_id: int,
         config: str,
         initial_view: GroupView
     )

**Parameters**:
  - ``engine``: Margo engine instance
  - ``provider_id``: Unique provider identifier
  - ``config``: JSON configuration string
  - ``initial_view``: Initial group membership

Backend Configurations
~~~~~~~~~~~~~~~~~~~~~~

Different backends suit different use cases:

.. literalinclude:: ../../../code/flock/13_python/server_backends.py
   :language: python
   :linenos:

**Static Backend** (lines 8-32):
  - Fixed membership set at initialization
  - Lightweight, no coordination overhead
  - Use for: Fixed-size groups, static deployments

**Centralized Backend** (lines 35-58):
  - Dynamic membership management
  - One provider coordinates membership
  - Use for: Dynamic groups, join/leave scenarios

Client-Side: Client API
------------------------

**Client Class**:
  .. code-block:: python

     from mochi.flock.client import Client

     # Initialize from address
     client = Client("tcp://hostname:port")

     # Or from existing engine
     client = Client(engine)

**Creating Group Handles**:

Three ways to create a GroupHandle:

1. **From address**:
   .. code-block:: python

      group = client.make_group_handle(
          address="tcp://server:1234",
          provider_id=42
      )

2. **From file**:
   .. code-block:: python

      group = client.make_group_handle_from_file("group.json")

3. **From serialized string**:
   .. code-block:: python

      group = client.make_group_handle_from_serialized(serialized_data)

GroupHandle Operations
~~~~~~~~~~~~~~~~~~~~~~

**Updating Group View**:
  .. code-block:: python

     group.update()  # Fetch latest membership from server

**Accessing View**:
  .. code-block:: python

     view = group.view  # Get current GroupView

**Properties**:
  .. code-block:: python

     group.client  # Get associated Client object

Working with GroupView
----------------------

GroupView represents group membership and metadata:

.. literalinclude:: ../../../code/flock/13_python/group_view_ops.py
   :language: python
   :linenos:

**Creating Views** (line 9):
  .. code-block:: python

     view = GroupView()

**Adding Members** (lines 13-15):
  .. code-block:: python

     view.members.add(address: str, provider_id: int)

**Accessing Members**:
  .. code-block:: python

     # Iterate
     for member in view.members:
         print(member.address, member.provider_id)

     # Index access
     first = view.members[0]

     # Size
     count = view.size

**Finding Members** (line 27):
  .. code-block:: python

     rank = view.members.find(address, provider_id)
     exists = view.members.contains(address, provider_id)

**View Properties**:
  .. code-block:: python

     digest = view.digest  # Unique hash of view
     metadata = view.metadata  # Dictionary of metadata

File Operations and Serialization
----------------------------------

Flock supports file-based bootstrapping and serialization:

.. literalinclude:: ../../../code/flock/13_python/file_operations.py
   :language: python
   :linenos:

**File-Based Configuration** (lines 24-31):
  Include ``"file"`` in config to write group info to a file:

  .. code-block:: python

     config = {
         "group": {
             "type": "static",
             "file": "/path/to/group.json"
         }
     }

**Loading from File** (line 41):
  .. code-block:: python

     group = client.make_group_handle_from_file(filename)

**Serialization** (line 46):
  .. code-block:: python

     serialized = group.view.to_json()
     group2 = client.make_group_handle_from_serialized(serialized)

This is useful for:
  - Passing group info between processes
  - Storing group configuration
  - Bootstrapping without a file

Group Updates
-------------

With centralized backend, you can update group membership:

.. literalinclude:: ../../../code/flock/13_python/group_updates.py
   :language: python
   :linenos:

**Updating** (line 44):
  .. code-block:: python

     group.update()  # Refresh view from server

This fetches the latest membership from the provider, reflecting any joins/leaves.

Context Managers
----------------

For automatic cleanup, use context managers:

.. code-block:: python

   import pymargo.core
   from mochi.flock.client import Client

   # Engine as context manager
   with pymargo.core.Engine("tcp", pymargo.core.client) as engine:
       client = Client(engine)
       # Client automatically cleaned up when engine exits

Common Patterns
---------------

**Service Discovery**:
  .. code-block:: python

     # Server writes group file
     config = {"group": {"type": "static", "file": "group.json"}}
     provider = Provider(engine, 42, json.dumps(config), view)

     # Clients discover via file
     client = Client(engine)
     group = client.make_group_handle_from_file("group.json")

**Dynamic Membership**:
  .. code-block:: python

     # Use centralized backend
     config = {"group": {"type": "centralized"}}
     provider = Provider(engine, 42, json.dumps(config), initial_view)

     # Clients can join
     # (join API depends on application-specific protocol)

     # Clients update to see new members
     group.update()

**Multiple Groups**:
  .. code-block:: python

     # Create multiple providers
     provider1 = Provider(engine, 1, config1, view1)
     provider2 = Provider(engine, 2, config2, view2)

     # Access different groups
     group1 = client.make_group_handle(address, 1)
     group2 = client.make_group_handle(address, 2)

Error Handling
--------------

Always check for errors and clean up resources:

.. code-block:: python

   import pymargo.core
   from mochi.flock.client import Client

   try:
       client = Client("tcp://localhost:1234")
       group = client.make_group_handle(address, provider_id)

       # Use group...

   except Exception as e:
       print(f"Error: {e}")
   finally:
       # Cleanup happens automatically via destructors
       # or use del explicitly
       del group
       del client

Integration with Other Mochi Services
--------------------------------------

Flock integrates with other Mochi Python APIs:

**With Bedrock**:
  .. code-block:: python

     from mochi.bedrock.server import Server
     from mochi.flock.client import Client

     # Bedrock can manage Flock providers
     server = Server(address="tcp")

     # Load config including Flock
     server.load_config(config_file)

     # Access Flock group
     client = Client(server.engine)
     group = client.make_group_handle(...)

**With Yokan** (using Flock for discovery):
  .. code-block:: python

     from mochi.flock.client import Client as FlockClient
     from mochi.yokan.client import Client as YokanClient

     # Discover Yokan servers via Flock
     flock = FlockClient(engine)
     group = flock.make_group_handle_from_file("servers.json")

     # Connect to Yokan on each member
     yokan = YokanClient(engine)
     for member in group.view.members:
         db = yokan.make_database_handle(
             member.address,
             member.provider_id
         )
         # Use database...

API Reference Summary
---------------------

**mochi.flock.server**:
  - ``Provider(engine, provider_id, config, initial_view)``

**mochi.flock.client**:
  - ``Client(arg)`` - arg can be Engine or address string
  - ``Client.make_group_handle(address, provider_id)``
  - ``Client.make_group_handle_from_file(filename)``
  - ``Client.make_group_handle_from_serialized(serialized)``

**mochi.flock.client.GroupHandle**:
  - ``update()`` - Refresh view from server
  - ``view`` - Get GroupView (property)
  - ``client`` - Get associated Client (property)

**mochi.flock.view.GroupView**:
  - ``size`` - Number of members
  - ``digest`` - Unique hash of view
  - ``members`` - MemberList object
  - ``metadata`` - Dictionary of metadata
  - ``to_json()`` - Serialize to JSON string

**GroupView.members (MemberList)**:
  - ``add(address, provider_id)``
  - ``find(address, provider_id)`` - Returns rank or -1
  - ``contains(address, provider_id)`` - Returns bool
  - ``__iter__()`` - Iterate over members
  - ``__getitem__(index)`` - Access by index
  - ``__len__()`` - Number of members

Next Steps
----------

- Explore Flock C++ API for advanced features (Tutorial 12)
- Learn about Bedrock integration (Tutorial 11)
- Understand different bootstrap methods (Tutorials 02-06)
- Study backend implementations (Tutorials 07-08)

The Python API provides a clean, Pythonic interface to Flock's powerful group
management capabilities, making it easy to build distributed Mochi applications
in Python.
