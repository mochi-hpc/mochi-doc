Python API
===========

This tutorial covers the Flock Python bindings, which provide a Pythonic interface
to Flock's group management capabilities. The Python API is built on top of the
C++ implementation and integrates seamlessly with pymargo.

Installation
------------

Install Flock with Python bindings using Spack:

.. code-block:: bash

   spack install mochi-flock +python

Quickstart Example
------------------

Here's a simple example showing the basics of Flock in Python:

.. literalinclude:: ../../../code/flock/13_python/quickstart.py
   :language: python
   :linenos:

Key Points
~~~~~~~~~~

**Imports:**

  The main modules are:
  - ``mochi.flock.server`` - Provider (server-side)
  - ``mochi.flock.client`` - Client and GroupHandle
  - ``mochi.flock.view`` - GroupView class
  - ``mochi.margo`` - Margo engine

**Creating a Provider:**

  .. code-block:: python

     provider = server.Provider(
         engine=engine,
         provider_id=42,
         config=json.dumps(config),
         initial_view=initial_view
     )

  The provider manages group membership. Configuration is passed as a JSON string.

**Creating a Client:**

  .. code-block:: python

     flock_client = client.Client(engine)

  Clients can connect to Flock providers to access group information.

**Creating a Group Handle:**
  .. code-block:: python

     group = flock_client.make_group_handle(
         address=str(engine.address),
         provider_id=42
     )

  A GroupHandle provides access to the group view.

Server-Side: Provider API
--------------------------

**Provider Class:**

  .. code-block:: python

     from mochi.flock.server import Provider

     provider = Provider(
         engine: pymargo.core.Engine,
         provider_id: int,
         config: str,
         initial_view: GroupView
     )

**Parameters:**

  - ``engine``: Margo engine instance
  - ``provider_id``: Unique provider identifier
  - ``config``: JSON configuration string
  - ``initial_view``: Initial group membership

Backend Configurations
~~~~~~~~~~~~~~~~~~~~~~

Different backends suit different use cases:

.. literalinclude:: ../../../code/flock/13_python/server_backends.py
   :language: python

**Static Backend:**

  - Fixed membership set at initialization
  - Lightweight, no coordination overhead
  - Use for: Fixed-size groups, static deployments

**Centralized Backend:**

  - Dynamic membership management
  - One provider coordinates membership
  - Use for: Dynamic groups, join/leave scenarios

Client-Side: Client API
------------------------

**Client Class:**

  .. code-block:: python

     from mochi.flock.client import Client

     # Initialize from address
     client = Client("ofi+tcp")

     # Or from existing engine
     client = Client(engine)

**Creating Group Handles:**

Three ways to create a GroupHandle:

1. **From address:**
   .. code-block:: python

      group = client.make_group_handle(
          address="...",
          provider_id=42
      )

2. **From file:**
   .. code-block:: python

      group = client.make_group_handle_from_file("group.json")

3. **From serialized string:**

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

File Operations and Serialization
----------------------------------

Flock supports file-based bootstrapping and serialization:

.. literalinclude:: ../../../code/flock/13_python/file_operations.py
   :language: python

**File-Based Configuration:**
  Include ``"file"`` in config to write group info to a file:

  .. code-block:: python

     config = {
         "group": {
             "type": "static"
         }
         "file": "/path/to/group.json"
     }

**Loading from File:**
  .. code-block:: python

     group = client.make_group_handle_from_file(filename)

This is useful for:

  - Passing group info between processes
  - Storing group configuration
  - Bootstrapping without a file

Group Updates
-------------

With centralized backend, you can update group membership:

.. literalinclude:: ../../../code/flock/13_python/group_updates.py
   :language: python

**Updating:**
  .. code-block:: python

     group.update()  # Refresh view from server

This fetches the latest membership from the provider, reflecting any joins/leaves.
