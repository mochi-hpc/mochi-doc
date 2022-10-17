Interoperability
================

Thallium and PyMargo are respectively C++ and Python wrappers for Margo,
hence, the three are interoperable. The table bellow summarizes the mapping
between Margo constructs and Thallium and PyMargo classes.


 ========================= ============================ =========================
  Margo                     Thallium                     PyMargo
 ========================= ============================ =========================
  margo_instance_id         engine                       Engine
  hg_addr_t                 endpoint                     Address
  hg_bulk_t                 bulk                         Bulk
  hg_handle_t (sender)      callable_remote_procedure    CallableRemoteFunction
  hg_handle_t (receiver)    request                      Handle
  margo_request             async_response               Request
 ========================= ============================ =========================


From Margo to Thallium
----------------------

The Thallium :code:`engine` and :code:`endpoint` classes provide
constructors that will create instances from their Margo counterparts.
To construct a Thallium :code:`bulk` from a Margo :code:`hg_bulk_t`,
you can use :code:`engine::wrap()`.  It is also possible to retrieve the
underlying Margo constructs from Thallium.
The :code:`engine::get_margo_instance()`,
:code:`endpoint::get_addr()`, and :code:`bulk::get_bulk()` functions return the
object's internal :code:`margo_instance_id`, :code:`hg_addr_t`, and
:code:`hg_bulk_t` respectively.

There is no conversion mechanism (but generally no need for them)
between :code:`hg_handle_t` and :code:`callable_remote_procedure` or
:code:`request`, and between :code:`async_response` and
:code:`margo_request`.

Serialization mechanisms differ between Margo and Thallium. Margo
uses either Mercury's boost preprocessor macros to automatically define
serialization function or rely on user-provided :code:`hg_proc` functions,
while Thallium relies on the Cereal library. Hence it is not possible to,
for instance, define an RPC using :code:`MARGO_REGISTER` on the client side,
and with :code:`thallium::engine::define()` on the server side, and vice versa.

.. important::
   If you are provided with a :code:`margo_instance_id` and wrap it in
   a Thallium :code:`engine` to define some RPC functions in C++, you will
   need your :code:`engine` instance to remain alive after having defined
   the RPC. This is because the RPC function retains a weak pointer to
   the engine's internal structure and will use it when it is invoked.
   This also applies when creating an instance of a provider: while
   Thallium's :code:`provider` class constructor  takes an :code:`engine`
   argument, it only keeps a weak reference to it.


From Margo to PyMargo
---------------------

PyMargo uses `pybind11 <https://pybind11.readthedocs.io>`_ to convert Margo
structures and functions into Python. More specifically, native C handles
are exposed as Python `capsules <https://docs.python.org/3/c-api/capsule.html>`_.
Higher-level classes then wrap these capsules to provide an object-oriented
interface for them. For instance, the :code:`Address` class is a pure-python
class wrapping a :code:`_pymargo.hg_addr_t` capsule, itself exposing an
:code:`hg_addr_t` native handle.

High-level Python classes generally have functions to get the internal
capsule of an object, should they be necessary to pass to C/C++ functions.
For instance, :code:`Address` has the :code:`hg_addr` property, :code:`Engine`
has an :code:`mid` property, and so on.

If you have a Mochi library developped in C or C++ and want to make a Python
binding for it that is compatible with PyMargo, we recommend that you look at
how `Yokan <https://github.com/mochi-hpc/mochi-yokan>`_ does it, in its `python`
folder.

Serialization in PyMargo relies on Python's `pickle` module, hence it is
not possible to register an RPC with :code:`MARGO_REGISTER` or :code:`engine::define()`
on one side, and expect Python to correctly work on the other side.

.. important::
   If you use Thallium as an intermediary between PyMargo and a C++ library,
   you can get a :code:`margo_instance_id` from a PyMargo :code:`Engine`,
   then create a Thallium :code:`engine` from it, but don't forget to keep this
   instance alive.
