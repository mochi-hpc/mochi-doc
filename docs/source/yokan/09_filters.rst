Key/value and document filters
==============================

The :code:`yk_list_*` and :code:`yk_doc_list_*` functions accept
a filter argument that can be used to only return key/value pairs
or document satisfying a certain condition. This tutorial shows how
to use such filters.

Key prefix and suffix filters
-----------------------------

By default, any filter data provided to the :code:`yk_list_*`
functions will be interpreted as a prefix that the *key* must start with.
If the :code:`YOKAN_MODE_SUFFIX` mode is used, it will be interpreted as
a suffix.

Lua key/value filters
---------------------

When using :code:`YOKAN_MODE_LUA_FILTER` is used, the content of the
filter will be interpreted as Lua code, which will be executed against
each key/value pair. Within such a code, the :code:`__key__`
variable will be a string set to the current key. If
:code:`YOKAN_MODE_FILTER_VALUE` is specified in the mode, the
:code:`__value__` variable will be set to the current value.
The Lua script must return a boolean indicating whether the pair
satisfies the user-provided condition.

Lua document filters
--------------------

For :code:`yk_doc_list_*` functions, Lua scripts will be provided with
the :code:`__id__` and :code:`__doc__` variables. The former is an integer
containing the document id. The latter is a string containing the document's
content.

Since document stores are often used to store JSON documents, Yokan already
integrates the *lua-cjson* library in its code, making it possible to convert
the :code:`__doc__` variable into a Lua hierarchy of tables:

.. code-block:: lua

   data = cjson.decode(__doc__)

Dynamic library filters
-----------------------

The :code:`YOKAN_MODE_LIB_FILTER` mode can use used to provide a filter
implemented in a shared library. The code bellow shows a custom key/value
filter and a custom document filter.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          filters.cpp (show/hide)

    .. literalinclude:: ../../../code/yokan/08_filters/filter.cpp
       :language: cpp


A custom key/value filter must inherit from :code:`yokan::KeyValueFilter`
and provide the following member functions.

- A constructor accepting a margo instance id, a mode, and filter data.
- :code:`requiresValue` should return whether the filter needs the value.
- :code:`check` runs the filter against a key/value pair.
- :code:`keyCopy` and :code:`valCopy` are used to copy the keys and values
  into a destination buffer. These functions can be used to extract or
  modify keys and values on the fly when they are read back by the user.
  They should return :code:`YOKAN_SIZE_TOO_SMALL` if the provided buffer
  size is too small for the data to copy.

The filter should be registered using the :code:`YOKAN_REGISTER_KV_FILTER`
macro, which takes the name of the filter, and the name of the class.

Once such a filter is provided, say in a library *libmy_custom_filter.so*,
:code:`yk_list_*` functions can be called with the following string as filter:
:code:`"libmy_custom_filter.so:custom_kv:..."`. Anything after the second
column will be interpreted as binary data and passed to the filter class
constructor's third argument, hence making it possible to provide arguments
to a custom filter.

The code above also shows a custom document filter. Such a filter must
provide the following member functions.

- A constructor accepting a margo instance id, a mode, and filter data.
- :code:`check` runs the filter against the document.

Similarly, the :code:`YOKAN_REGISTER_DOC_FILTER` macro should be used
to register the filter.

There is currently no :code:`docCopy` function in document filters, so
documents cannot be modified on the fly when read.
