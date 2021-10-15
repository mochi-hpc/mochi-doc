C++ and Python bindings
=======================

Yokan provides C++ and Python bindings.

C++ binding
-----------

C++ bindings are headers-only and located in *include/yokan/cxx/*.
The API is pretty much self-explanatory: it provides C++ class
equivalent to the corresponding C opaque pointers and identifiers.
For instance the ``yokan::Client`` class can be used in place of
the `yk_client_t` C equivalent.

.. warning::
   Some C++ functions may have their parameters in a different
   order than the corresponding C function (in particular all
   the functions that take a mode have this mode as last
   parameter). This is to allow C++ optional parameters.

Python binding
--------------

Yokan also provides Python bindings for its server, admin, and client
libraries. The best way to understand their use is to look in the
*tests/python* folder. To enable this python binding, install Yokan
with the ``+python`` variant in spack.

The Python binding aims to encourage the user to think about performance
first, before thinking about usability. We encourage users to implement
a higher-level API on top of Yokan's API to suit their particular user-case.
For instance, the Python API does not provide ``__iter__`` capabilities to
enable seemless iteration through key/value pairs. This kind of functionality
would need to be implemented by the user using the ``list_*`` function,
with the added knowledge of desired batch sizes, buffer sizes, etc.

.. important::
   All the functions in the Python binding take either a string, or
   any object that implements the `buffer protocol <https://docs.python.org/3/c-api/buffer.html>`_.
   Such objects include for instance ``bytearray`` and numpy arrays.
   Using objects satisfying the buffer protocol will be more efficient
   than using strings because the latter causes memory copies to occur
   when the Python string is converted into a C++ string internally.
