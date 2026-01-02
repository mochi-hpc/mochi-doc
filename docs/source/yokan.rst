Yokan (Key/Value storage component)
=======================================

Yokan is a Mochi microservice that provides single-node key/value and document storage
capabilities. It is based on Margo, and offers many backends, including RocksDB,
LevelDB, BerkeleyDB, LMDB, GDBM, Unqlite, and TKRZW.
Yokan has been designed to offer as much flexibility as possible. Many of its
internal components can be configured, from the storage backends to how buffers
are allocated and cached. It is also easy to extend with new backends, should
the user need specific capabilities, or for research purposes.

This section will walk you through a series of tutorials on how to use Yokan.

.. toctree::
   :maxdepth: 1

   yokan/01_init.rst
   yokan/02_advanced_setup.rst
   yokan/03_basics.rst
   yokan/04_multi.rst
   yokan/05_modes.rst
   yokan/06_docs.rst
   yokan/07_backends.rst
   yokan/08_bindings.rst
   yokan/09_filters.rst
   yokan/10_migration.rst
   yokan/11_watcher.rst
   yokan/12_python.rst
   yokan/13_cpp.rst
