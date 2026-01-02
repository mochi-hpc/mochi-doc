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

.. literalinclude:: ../../code/yokan/11_watcher/wait_notify_basic.cpp
   :language: cpp

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

.. literalinclude:: ../../code/yokan/11_watcher/wait_notify_multi.cpp
   :language: cpp

This is useful for broadcast-style notifications where multiple workers
need to be triggered by the same event.

Producer/Consumer Pattern
-------------------------

A common use case is implementing producer/consumer queues where consumers
wait for producers to provide data:

.. literalinclude:: ../../code/yokan/11_watcher/producer_consumer.cpp
   :language: cpp

This example demonstrates:

- Multiple producers creating work items
- Multiple consumers waiting for and processing items
- Using key naming to create a work queue
- Consuming items with ``YOKAN_MODE_CONSUME`` to prevent duplicate processing

Distributed Coordination
------------------------

Wait/notify can coordinate distributed processes across different nodes:

.. literalinclude:: ../../code/yokan/11_watcher/distributed_coord.cpp
   :language: cpp

This pattern is useful for:

- Distributed barriers
- Checkpoint synchronization
- Multi-stage pipelines
- Leader election

Without Wait/Notify
--------------------

For comparison, here's what coordination looks like without wait/notify,
using polling instead:

.. literalinclude:: ../../code/yokan/11_watcher/without_wait.cpp
   :language: cpp

The polling approach:

- Wastes CPU and network resources
- Has unpredictable latency based on poll interval
- Doesn't scale well with many waiters
- Can miss rapid updates

The wait/notify approach is more efficient and responsive.

Error Handling and Timeouts
----------------------------

Wait operations can be interrupted when the provider shuts down:

.. literalinclude:: ../../code/yokan/11_watcher/wait_timeout.cpp
   :language: cpp

Best practices for wait/notify:

1. **Handle shutdown gracefully**: Waiting operations may fail if the
   provider shuts down while clients are waiting

2. **Use unique keys**: Ensure each notification has a unique key to
   avoid accidental wake-ups

3. **Clean up consumed keys**: Use ``YOKAN_MODE_CONSUME`` or explicit
   erase operations to prevent keys from accumulating

4. **Consider backend support**: Verify your backend supports these modes

5. **Combine with other modes**: You can combine wait/notify with other
   modes like ``YOKAN_MODE_CONSUME`` or ``YOKAN_MODE_APPEND``

Bedrock Integration
-------------------

When using Yokan with Bedrock, ensure your backend configuration supports
wait/notify:

.. literalinclude:: ../../code/yokan/11_watcher/bedrock-config.json
   :language: json

The ``map`` backend fully supports wait/notify operations. Other backends
may have varying levels of support - consult the backend documentation.

Performance Considerations
--------------------------

The wait/notify mechanism:

- **Efficient**: No polling overhead, minimal network traffic
- **Scalable**: Supports many concurrent waiters
- **Low latency**: Immediate notification when keys appear
- **Thread-safe**: Multiple threads can safely wait/notify

However, keep in mind:

- Waiting operations hold server resources until notified or interrupted
- Very long waits may impact server memory if many clients are waiting
- Consider application-level timeouts for long-running waits

Next Steps
----------

- Learn about :doc:`12_python` for Python bindings
- Explore :doc:`13_cpp` for advanced C++ patterns
- Review :doc:`05_modes` for other available modes
- See :doc:`10_migration` for data migration

Summary
-------

The wait/notify feature provides:

- Efficient event-driven coordination
- Reduced polling overhead
- Support for producer/consumer patterns
- Distributed synchronization primitives

By using ``YOKAN_MODE_WAIT`` and ``YOKAN_MODE_NOTIFY``, you can build
responsive, scalable distributed applications without the overhead of
traditional polling approaches.
