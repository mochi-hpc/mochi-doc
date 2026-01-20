Event Notifications with Wait/Notify
====================================

Yokan provides a powerful event notification mechanism through the
``YOKAN_MODE_WAIT`` and ``YOKAN_MODE_NOTIFY`` modes. This allows clients
to coordinate asynchronously by waiting for specific keys to appear in
the database and being notified when they do.

This feature is particularly useful for:

- Producer/consumer patterns
- Distributed coordination
- Event-driven workflows
- Barrier synchronization
- Pipeline processing

Basic Concepts
--------------

The wait/notify mechanism works as follows:

1. **Waiting clients** use ``YOKAN_MODE_WAIT`` when performing get operations
   on keys that don't yet exist. Instead of returning an error, the operation
   blocks until the key appears.

2. **Notifying clients** use ``YOKAN_MODE_NOTIFY`` when putting values.
   This wakes up any clients waiting for those specific keys.

The key advantage is that waiting clients don't need to poll the database
repeatedly - they simply block until the data is available, reducing
network traffic and improving efficiency.

.. important::

   Not all backends support wait/notify modes. If you attempt to use
   these modes with an unsupported backend, you'll receive a
   ``YOKAN_ERR_MODE`` error. The in-memory backends (``map``, ``unordered_map``)
   support this feature.

Basic Wait/Notify Example
--------------------------

Here's a simple example demonstrating the wait/notify pattern:

.. literalinclude:: ../../../code/yokan/11_watcher/wait_notify_basic.c
   :language: c

In this example:

- The **consumer** starts waiting for a key that doesn't exist yet
- The **producer** puts the value with ``YOKAN_MODE_NOTIFY``
- The consumer's ``get()`` operation completes and receives the value

The consumer thread blocks at the ``get()`` call until the producer
puts the value, providing efficient event-driven coordination.

Multiple Waiters
----------------

Multiple clients can wait for the same key. When a put operation
uses ``YOKAN_MODE_NOTIFY``, all waiting clients are woken up:

.. literalinclude:: ../../../code/yokan/11_watcher/wait_notify_multi.c
   :language: c

This is useful for broadcast-style notifications where multiple workers
need to be triggered by the same event.

Producer/Consumer Pattern
-------------------------

A common use case is implementing producer/consumer queues where consumers
wait for producers to provide data:

.. literalinclude:: ../../../code/yokan/11_watcher/producer_consumer.c
   :language: c

This example demonstrates:

- Multiple producers creating work items
- Multiple consumers waiting for and processing items
- Using key naming to create a work queue
- Consuming items with ``YOKAN_MODE_CONSUME`` to prevent duplicate processing

Distributed Coordination
------------------------

Wait/notify can coordinate distributed processes across different nodes:

.. literalinclude:: ../../../code/yokan/11_watcher/distributed_coord.c
   :language: c

This pattern is useful for:

- Distributed barriers
- Checkpoint synchronization
- Multi-stage pipelines
- Leader election

Without Wait/Notify
--------------------

For comparison, here's what coordination looks like without wait/notify,
using polling instead:

.. literalinclude:: ../../../code/yokan/11_watcher/without_wait.c
   :language: c

The polling approach:

- Wastes CPU and network resources
- Has unpredictable latency based on poll interval
- Doesn't scale well with many waiters
- Can miss rapid updates

The wait/notify approach is more efficient and responsive.

Timeouts
--------

There is currently no way to timeout a watcher; the watcher *will* need
to be notified by a writer.
