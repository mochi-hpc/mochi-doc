Changing API semantics with modes
=================================

You may have noticed the :code:`YOKAN_MODE_DEFAULT` parameter
passed to all client functions. This argument can be used
to alter the semantics of the function. You may use multiple such
modes by chaining them using bitwise "or".

The flags currently available are listed hereafter.

- ``YOKAN_MODE_DEFAUL``: Default mode.
- ``YOKAN_MODE_INCLUSIVE``: The lower bound key in :code:`list`
  functions will be included in the result if found.
- ``YOKAN_MODE_APPEND``: :code:`put` operations will append
  to values instead of overwriting them.
- ``YOKAN_MODE_CONSUME``: :code:`get` operations will also
  erase the accessed key/value pairs.
- ``YOKAN_MODE_WAIT``: The operation will wait for the key to appear
  in the database instead of returning an error. Writers putting
  the key will need to use the ``YOKAN_MODE_NOTIFY`` mode.
- ``YOKAN_MODE_NOTIFY``: See above.
- ``YOKAN_MODE_NEW_ONLY``: Only put the key/value pair if the key did
  not exist.
- ``YOKAN_MODE_EXIST_ONLY``: Only put the key/value pair if the key
  was already in the database.
- ``YOKAN_MODE_SUFFIX``: In :code:`list_keys(_packed)` and :code:`list_keyvals(_packed)`,
  the filter argument is considered a prefix by default. This mode makes the function
  consider it as a suffix.
- ``YOKAN_MODE_NO_PREFIX``: In :code:`list` operations, remove the
  specified prefix (or suffix, if ``YOKAN_MODE_SUFFIX`` is used) from the
  resulting keys.
- ``YOKAN_MODE_IGNORE_KEYS``: In :code:`list_keyvals`, do not send
  keys back, only send values.
- ``YOKAN_MODE_KEEP_LAST``: In :code:`list_keyvals`, in conjunction
  with YOKAN_MODE_IGNORE_KEYS, still send the last key.
- ``YOKAN_MODE_LUA_FILTER``: Interpret the filter argument in :code:`list_keys(_packed)`
  and :code:`list_keyvals(_packed)` as Lua code.
- ``YOKAN_MODE_IGNORE_DOCS``: Will make :code:`yk_doc_list` and :code:`yk_doc_list_packed`
  only return the ids of documents satisfying the provided filter.
- ``YOKAN_MODE_FILTER_VALUE``: This mode must be specified if the provided filter
  requires the value. If not specified, some backends may elect to call the filter
  function with a null value in place of the actual value.
- ``YOKAN_MODE_LIB_FILTER``: Loads a custom filter from a shared library.
  See the tutorial on filters for more information.
- ``YOKAN_MODE_NO_RDMA``: Will make functions switch to a version of the RPCs
  that don't use RDMA. Data will be packed within the RPC request and response.
  While this can cause more copies than necessary, it can still be more efficient
  in particular for small key/value pairs and small documents. Note that not all
  the functions currently support this mode, it is simply considered as a hint.

.. important::

   Not all backends support all modes, and not all the client functions support
   all modes. If a backend doesn't
   support a particular mode, the function will return :code:`YOKAN_ERR_MODE`.
   If a backend doesn't support a mode that you need, and you really want
   to use this backend, please let us know and we will try to add support for
   it. Similarly if you think of a mode that would be useful, let us know.
