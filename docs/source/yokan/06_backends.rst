Backends and their configurations
=================================

So far we have used the ``map`` backend for this tutorial.
Yokan provides a number of backends, listed bellow, along
with some key information on their configuration.

Note that the full configuration format of a backend can
be known by creating a database using the backend in a Bedrock
context, then using the :ref:`bedrock-query` tool to retrieve the
full configuration.

Some configuration parameters expect a symbol name that will
be looked up using ``dlsym` and ``dlopen``. These names can be written
in the form ``"symbol_name"`` to look them up in the currently
loaded executable and libraries, or ``"libsome_library.so:symbol_name"``
to look them up in an exernal library that needs to be loaded.

Map backend
-----------

- Backend type: "map"
- Spack variant needed: none
- Special requirements: none

The Map backend is an in-memory key/value store implemented
using a C++ ``std::map``. Important configuration parameters
include the following.

- ``use_lock``: true by default, this parameter can be set
  to false if you know that the database will never be accessed
  by multiple execution stream concurrently (either because only
  one client accesses it, or because only one execution stream
  services RPC for it).
- ``comparator``: a function with signature
  ``bool (*)(const void*, size_t, const void*, size_t)`` that
  provides the *"less than"* comparison operator for two keys.
- ``allocators``: a JSON object containing the ``key_allocator``,
  ``value_allocator``, and ``node_allocator`` parameters, along
  with corresponding ``key_allocator_config``, ``value_allocator_config``,
  and ``node_allocator_config``.

The memory allocators used by the Map backend can be customized
using the ``allocators`` configuration. The keys and values allocator
are used to allocate memory to store the keys and values respectively.
The node allocator is used to allocate memory for the nodes in the map's
underlying red-black tree. By default, an allocator that uses ``new`` and
``delete`` will be used, but if you know in advance that, for instance,
the keys or the values are going to be of a constant size, or that there
will be no deletion from the database, or that the database won't be accessed
concurrently by multiple threads, you can provide your own allocators
accordingly.

By default the ``*_allocator`` fields are set to "default".
To implement a custom allocator, write a dynamic library implementing
functions from the ``yk_allocator`` structure (found in *include/yokan/allocator.h*),
along with an initialization function of type ``yk_allocator_init_fn``.
You can then set these fields to ``"libmy_allocators.so:my_key_alloc_init"``,
for example. Note that the initialization functions' second argument is a
``const char*``. It will be passed a serialized version of the JSON object
stored in the corresponding ``*_config`` field (e.g. the key allocator
initialization function will be provided with the content of the "key_allocator_config"
field).

Unordered Map backend
---------------------

- Backend type: "unordered_map"
- Spack variant needed: none
- Special requirements: none

The Unordered Map backend is similar to the Map backend but relies
on C++'s ``std::unordered_map``. It has the same configuration fields.

Contrary to the Map backend, this backend does not support the ``list_*``
functions. This backend aims to provide faster lookup by relying on a hash
table rather than Map's red-black tree, at the expense of not supporting
key ordering.

Set backend
-----------

- Backend type: "set"
- Spack variant needed: none
- Special requirements: none

The Set backend uses C++'s ``std::set``. It is similar to the Map backend
but does not store values. Trying to put anything else than zero-sized values
will result in an error. This backend is useful over the Map backend in that
even if you were to store only zero-sized values in the Map backend, you would
still have to store a ``size_t`` (generally 8 bytes) field for each value.

This backend's configuration is similar to that of the Map backend, but
without the allocator fields related to values.

Unordered Set backend
---------------------

- Backend type: "unordered_set"
- Spack variant needed: none
- Special requirements: none

This backend is the Unordered Map equivalent for Sets. It stores only
zero-sized values, and does not provide ordering, hence ``list_*`` functions
are not available.

BerkeleyDB backend
------------------

- Backend type: "berkeleydb"
- Spack variant needed: +berkeleydb
- Special requirements: none

This backend uses `BerkeleyDB <https://www.oracle.com/uk/database/technologies/related/berkeleydb.html>`_
to implement a key/value store backed by
a local file system. Important configuration fields include the following.

- ``type``: may be "btree" or "hash". The former provides a sorted.
  key/value store, while the latter is unsorted and will not provide
  the ``list_*`` operations.
- ``home``: path to the "home" of the BerkeleyDB environment (a "yokan"
  subdirectory will be added to this path).
- ``file``: name of the file storing the database.
- ``name``: name of the database.
- ``create_if_missing``: create the files if they are not present at
  the specified location in the file system.

If the "file" and "name" fields are empty or not provided, the
database will be stored in memory rather than files.

GDBM backend
------------

- Backend type: "gdbm"
- Spack variant needed: +gdbm
- Special requirements: none

This backend uses `GDBM <https://www.gnu.org.ua/software/gdbm/>`_,
a widely use database management library for
Unix systems. Important configuration fields include the following.

- ``path``: path to the database file.

This backend does not provide the ``list_*`` functionalities.

LevelDB backend
---------------

- Backend type: "leveldb"
- Spack variant needed: +leveldb
- Special requirements: none

This backend uses Google's `LevelDB <https://github.com/google/leveldb>`_
to provide key/value storage capabilities.
Important configuration fields include the following.

- ``path``: path to the database file.
- ``error_if_exists``: fail to open the database if it already exists.
- ``create_if_missing``: fail to open the database if it does not exist.

LMDB backend
------------

- Backend type: "lmdb"
- Spack variant needed: +lmdb
- Special requirements: none

This backend uses `LMDB <http://www.lmdb.tech/doc/>`_ to provide
key/value storage capabilities.
Important configuration fields include the following.

- ``path``: path to the database file.
- ``create_if_missing``: create the file if it is missing.
- ``no_lock``: disable locking.

LMDB uses its own locks internally, which are not Argobots-aware.
The ``no_lock`` option disables this internal locking, but as
of now, Argobots locks were not added to compensate. Hence, use
this option only if you know that database accesses will be serialized
(either because only one ES accesses it, or because only one client
accesses it, in a serial manner).

RocksDB backend
---------------

- Backend type: "rocksdb"
- Spack variant needed: +rocksdb
- Special requirements: none

This backend uses Facebook's `RocksDB <http://rocksdb.org/>`_ for key/value storage.
Important configuration fields include the following.

- ``create_if_missing``: create the database if it does not exist.
- ``error_if_exists``: fail to open the database if it exists.
- ``path``: path to the database.
- ``db_paths``: an array of JSON objects representing storage targets
  to use to store the database files. Each such an object should
  have a ``path`` field and a ``target_size`` field.

TKRZW backend
-------------

- Backend type: "tkrzw"
- Spack variant needed: +tkrzw
- Special requirements: compiler allowing C++17

The `TKRZW <https://dbmx.net/tkrzw/>`_ library provides multiple types of backends.
Important configuration fields include the following.

- ``type``: may be "tree", "hash", "tiny", or "baby". The first is a
  typical tree-based key/value store backed up by a file. The second is
  a hash-based key/value store (no ordering, so no ``list_*`` operations)
  backed up by a file. The latter two are in-memory versions of the former.
- ``path``: path to the database file, if relevant given the ``type``.

Unqlite backend
---------------

- Backend type: "unqlite"
- Spack variant needed: +unqlite
- Special requirements: none

`Unqlite <https://unqlite.org/>`_ is a document store targetting JSON documents. However its
key/value storage capabilities are accessible natively. This backend
ditches the "document store" aspect and simply relies on they key/value
storage capabilities.

This backend is unsorted and does not provide ``list_*`` operations.

Important configuration fields include the following.

- ``path``: path to the database file.
- ``mode``: either "create", "read_write", "read_only", "mmap", or "memory".
- ``temporary``: will erase the database file upon closing.
- ``use_abt_lock``: protect database accesses using Argobots locks.
- ``no_unqlite_mutex``: disable the use of mutex inside Unqlite.

Mutex used by Unqlite are not Argobots-aware. It may therefore make sense
to disable them and enable ``use_abt_lock`` for better performance.


Writing your own backend
========================

Yokan aims to provide an easy way for researchers to implement and try
their own backend key/value stores. To implement your own key/value
storage backend, look at *include/yokan/backend.hpp* and implement
a child class of the ``KeyValueStoreInterface`` abstract class.
You can take inspiration from *src/backends/map.cpp* to understand
the semantics of each member function.

Once your backend is implemeted in a .cpp file, use the
``YOKAN_REGISTER_BACKEND`` macro, e.g. ``YOKAN_REGISTER_BACKEND(mybackend, MyBackend)``.
Its first argument is the name you want to give to the backend
(i.e. the type that will be used in configuration files).
The second argument is the name of your backend class.

Compile your .cpp file into a dynamic library. Then, when
specifying the type of a database, use the syntax "library.so:name",
there "library.so" is your dynamic library, and "name" is the name of
your backend.

Note that Yokan will use ``dlopen`` to load the library, so its lookup
rules apply (for instance ``dlopen`` will usually look for libraries
in the ``LD_LIBRARY_PATH`` environment variable).
