C++ and Python Bindings
=======================

Yokan provides comprehensive C++ and Python bindings for building applications
in modern C++ and Python.

Quick Overview
--------------

- **C++ bindings**: Header-only, modern C++ API with RAII and exceptions
- **Python bindings**: Full-featured Python API with buffer protocol support
- Both bindings provide complete access to all Yokan features

For detailed documentation and examples, see the dedicated tutorials:

.. toctree::
   :maxdepth: 1

   12_python.rst
   13_cpp.rst

C++ Bindings Summary
--------------------

C++ bindings are header-only and located in ``include/yokan/cxx/``.
They provide:

- RAII resource management (automatic cleanup)
- Exception-based error handling
- Modern C++ idioms
- Full API coverage

Key classes:

- ``yokan::Client`` - Equivalent to ``yk_client_t``
- ``yokan::Database`` - Equivalent to ``yk_database_handle_t``
- ``yokan::Exception`` - For error handling

.. warning::
   Some C++ functions have parameters in a different order than their C
   equivalents. In particular, functions that take a mode have this mode
   as the last parameter to allow C++ optional parameters.

**→ See** :doc:`13_cpp` **for complete C++ documentation and examples**

Python Bindings Summary
-----------------------

Yokan provides Python bindings for both server and client libraries.
To enable Python bindings, install Yokan with the ``+python`` variant in Spack:

.. code-block:: bash

   spack install mochi-yokan+python

The Python bindings provide:

- Server API (``mochi.yokan.server``)
- Client API (``mochi.yokan.client``)
- Full backend support
- Mode system
- Document collections

Performance-Oriented Design
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Python bindings prioritize performance while maintaining ease of use.
We encourage users to build higher-level APIs on top of Yokan's API to suit
their specific use cases.

.. important::
   All functions in the Python bindings accept either strings or any object
   that implements the `buffer protocol <https://docs.python.org/3/c-api/buffer.html>`_.
   Objects like ``bytearray`` and NumPy arrays are more efficient than strings
   because they avoid memory copies.

**→ See** :doc:`12_python` **for complete Python documentation and examples**

Quick Start Examples
--------------------

Python Quick Start
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   from mochi.margo import Engine
   from mochi.yokan.server import Provider
   from mochi.yokan.client import Client

   engine = Engine('tcp')
   provider = Provider(engine=engine, provider_id=42,
                      config='{"database":{"type":"map"}}')
   client = Client(engine=engine)
   db = client.make_database_handle(address=engine.addr(), provider_id=42)

   db.put(key="greeting", value="Hello, Yokan!")
   value = db.get(key="greeting")
   print(f"Retrieved: {value}")

   engine.finalize()

C++ Quick Start
~~~~~~~~~~~~~~~

.. code-block:: cpp

   #include <yokan/cxx/client.hpp>
   #include <yokan/cxx/database.hpp>

   int main() {
       yokan::Client client("na+sm");
       yokan::Database db = client.makeDatabaseHandle(
           "na+sm://localhost:1234", 42);

       std::string key = "greeting";
       std::string value = "Hello, Yokan!";
       db.put(key.data(), key.size(), value.data(), value.size());

       std::vector<char> buffer(256);
       size_t vsize = buffer.size();
       db.get(key.data(), key.size(), buffer.data(), &vsize);

       return 0;
   }

Next Steps
----------

- **For Python users**: Read :doc:`12_python` for comprehensive Python documentation
- **For C++ users**: Read :doc:`13_cpp` for comprehensive C++ documentation
- **For C users**: Continue with the C API tutorials in this section

The dedicated tutorials include:

- Complete API references
- Practical examples
- Best practices
- Performance tips
- Integration patterns
