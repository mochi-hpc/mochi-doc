Accessing multiple key/value pairs
==================================

Each of the functions presented in the previous tutorial
leads to one RPC sent to the provider. Hence, batching can
be helpful to optimize performance by accessing multiple
key/value pairs in a single RPC.

Yokan provides two ways of accessing batches of key/value pairs:
*packed*, and *unpacked*, with function signatures varying
slightly depending on which way is used.

Unpacked accesses can be used when lists of keys and values
are not located in a contiguous memory location. Functions
using unpacked accesses will generally have the :code:`_multi`
suffix, and take *arrays of pointers*.

Packed accesses can be used when keys and values are packed
contiguously in memory. Functions using packed accesses will
have the :code:`_packed` suffix, and will take *pointers to
single buffers* instead of arrays of pointers.

.. important::

   In general, it is recommended to use packed accesses as much
   as possible, even if this means bouncing data through an
   intermediate buffer. Unpacked accesses should be reserved
   for cases where (1) such a copy would be costly (e.g. large
   key/value pairs) and (2) you know the size of the keys and
   values (or at least a very close upper bound) ahead of time.


Unpacked accesses
-----------------

The following program gives you an example of usage for all
unpacked functions.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/yokan/04_multi/client-multi.c
       :language: cpp

These functions are the following.

- :code:`yk_put_multi`: puts multiple key/value pairs into the database.
- :code:`yk_exists_multi`: checks if multiple keys exist in the database.
  The :code:`uint8_t flags[]` output parameter is a bitfield, hence for N
  keys, its size should be :code:`int((N+8)/8)`. The bitfield can be
  interpreted using :code:`yk_unpack_exists_flag`.
- :code:`yk_length_multi`: retrieves the length of the values associated
  with a set of keys.
- :code:`yk_get_multi`: retrieves the values associated with a set of keys.
- :code:`yk_fetch_multi`: retrieves the values associated with a set of keys.
- :code:`yk_list_keys`: lists a specified number of keys, starting after
  the specified lower bound.
- :code:`yk_list_keyvals`: same as :code:`yk_list_keyvals` but also returns
  values.
- :code:`yk_iter`: another way of iterating through key/value pairs, using
  a callback. This method can improve performance by avoiding memory copies.
  It also lets the Yokan client manage and reuse buffers, and pipeline
  calls to fetch the next batch of key/value pairs.

Once again, more documentation on the semantics of all these functions
is available in the *yokan/database.h* header.

Packed accesses
---------------

The following program gives you an example of usage for all
packed functions.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          client.c (show/hide)

    .. literalinclude:: ../../../code/yokan/04_multi/client-packed.c
       :language: cpp

These functions are the following.

- :code:`yk_put_packed`: puts multiple key/value pairs into the database.
- :code:`yk_exists_packed`: checks if multiple keys exist in the database.
- :code:`yk_length_packed`: retrieves the length of the values associated
  with a set of keys.
- :code:`yk_get_packed`: retrieves the values associated with a set of keys.
- :code:`yk_list_keys_packed`: lists a specified number of keys, starting after
  the specified lower bound.
- :code:`yk_list_keyvals_packed`: same as :code:`yk_list_keyvals` but also returns
  values.

Note that some of the semantics of these functions differ slighly from that
of their :code:`_multi` counterpart. Functions that retrieve data (:code:`yk_get_packed`,
:code:`yk_list_keys_packed`, and :code:`yk_list_keyvals_packed`) do not expect
you to provide the size of individual items (apart from the key sizes in
:code:`yk_get_packed`), but require you to provide a total buffer size.
The functions will then try to fill this buffer. This is especially useful
when keys and values have varying sizes that are not known in advance, as
you don't need to query their sizes first.
