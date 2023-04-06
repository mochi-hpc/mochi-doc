Bake's client interface
=======================

A simple client example
-----------------------

The following code shows how to use the Bake client library to access
a target on a remote Bake provider.

.. literalinclude:: ../../../code/bake/02_client/client.c
   :language: c

The :code:`bake_client_init` function is used to create a :code:`bake_client`
object. This client object is required to then create provider handles using
:code:`bake_provider_handle_create`.

In this example, the target ID is either passed as the program's second argument,
in which case :code:`bake_target_id_from_string` is used to decode it into a
:code:`bake_target_id_t` object, or it is queried from the provider using the
:code:`bake_probe` function. This function takes the provider handle, the maximum
number of targets to list, a pointer to an array of target IDs (here just one target),
and a pointer to store the returned numbed of targets.

The :code:`bake_create` function is used to create a region of 10 bytes in the target.
It returns a :code:`bake_region_id_t` identifying this region. This region ID can be
used to write into the region using :code:`bake_write`, or to read from it using
:code:`bake_read`. Note that in our example, we write and read the full 10 bytes of
the region, starting from offset 0, but this is not a requirement. These operations
may access any part of the region using different offsets and sizes.

.. important::
   Unless Bake was built with the :code:`+sizecheck` option, read and write operations
   will not check that the provided offset and size fall within the requested region.
   This can lead to corruption of the target if the region is accessed outside of its
   bounds.

:code:`bake_write` does not guarantee that the written data is persistent on the
target (e.g., should the service crash after the write, the data may not be there
upon restarting the service). To ensure persistence or a given segment within a
given region, one has to call :code:`bake_persist`.

.. note::
   Creating a new region, writing the entirety of its content, and persisting it,
   is a very common pattern. Hence Bake provides the :code:`bake_create_write_persist`
   function for this purpose.


Other client functions
----------------------

The Bake client library provides a number of other functions that may be useful.

On provider handles:

* :code:`bake_provider_handle_ref_incr` can be used to increase the internal
  reference count of the provider handle.
* :code:`bake_provider_handle_get_info` retrieves the provider handle's internal
  information (client, address, and provider id).
* :code:`bake_provider_handle_get/set_eager_limit` access the provider handle's
  "eager limit". This value is the number of bytes under which the client will
  send data using RPC arguments instead of RDMA transfer.

On regions:

* :code:`bake_get_size` gets the size of a given region.
* :code:`bake_get_data` can be used if the caller is in the same process as the
  target bake provider, to obtain a direct pointer to the region's data. Using
  this function is however not recommended since it strongly couples the client
  code with the server code (forcing them to live in the same process space).
* :code:`bake_remove` can be used to remove a region. Note that not all backends
  support removing a region.
* :code:`bake_proxy_write` and :code:`bake_create_write_persist_proxy` are versions
  of :code:`bake_write` and :code:`bake_create_write_persist` that take a bulk handle,
  a remote address (as a string) and an offset, as the source of the data to be
  written into the region. These functions are useful in two scenarios: (1) when
  the data comes from another process and the calling process is only forwarding
  a bulk handle to the Bake server; (2) if the program needs to send data that is
  not contiguous in memory, by creating a bulk handle for it manually.
* :code:`bake_proxy_read` can similarly be used to read the content of a region
  into a bulk handle instead of a buffer.
