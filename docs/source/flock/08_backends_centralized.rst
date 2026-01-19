Backend: centralized
====================

The centralized backend provides dynamic group management with a centralized
coordinator. It supports membership changes, fault detection, and notifications.

When to use
-----------

Use the centralized backend when:

- Group membership changes over time (members join/leave)
- You need fault tolerance and failure detection
- You want membership change notifications
- You're building elastic services that scale dynamically

Characteristics
---------------

**Dynamic membership**: Members can join and leave the group after initialization.

**Centralized coordination**: One member acts as the coordinator, managing the
authoritative group view.

**Failure detection**: Can detect and handle member failures.

**Notifications**: Members can subscribe to membership change events.

**Higher overhead**: More resource usage than static backend due to coordination.

Configuration
-------------

In Bedrock configuration:

.. code-block:: json

   {
       "providers": [
           {
               "type": "flock",
               "provider_id": 42,
               "config": {
                   "bootstrap": "self",
                   "group": {
                       "type": "centralized",
                       "config": {
                           "heartbeat_interval_ms": 5000,
                           "failure_timeout_ms": 15000
                       }
                   }
               }
           }
       ]
   }

Configuration options:

- :code:`heartbeat_interval_ms`: How often members send heartbeats (default: 5000ms)
- :code:`failure_timeout_ms`: Time before a member is considered failed (default: 15000ms)

In C code
---------

.. literalinclude:: ../../../code/flock/08_backends_centralized/server.c
   :language: c

How it works
------------

The centralized backend operates as follows:

**Coordinator selection**: The first member to initialize becomes the coordinator.

**Heartbeats**: All members periodically send heartbeats to the coordinator.

**Join protocol**:

1. New member contacts coordinator
2. Coordinator adds member to its view
3. Coordinator notifies other members of the new member
4. New member receives the updated view

**Leave/Failure protocol**:

1. Coordinator detects missing heartbeats
2. After timeout, member is marked as failed
3. Coordinator updates the view
4. Other members are notified of the change

Dynamic membership
------------------

Unlike the static backend, the centralized backend supports adding and removing members:

**Adding members**:

Use the "join" bootstrap method to add members to a running group.

**Removing members**:

Members are automatically removed when they:

- Stop sending heartbeats (failure detection)
- Call :code:`flock_provider_deregister` (graceful shutdown)

Coordinator resilience
----------------------

The coordinator is a single point of failure. If the coordinator fails:

1. Heartbeats from other members will timeout
2. Members will detect the coordinator failure
3. A new coordinator can be elected (future feature)

Currently, if the coordinator fails, the group becomes unavailable. Best practices:

- Run the coordinator on a reliable node
- Monitor the coordinator's health
- Have a backup plan for coordinator failure

Example: Elastic service
------------------------

Here's an example of building an elastic service that can scale up dynamically:

**Initial coordinator**:

.. code-block:: console

   $ ./server
   Flock provider registered with CENTRALIZED backend
   Group membership can change dynamically
   Update interval: 1000ms
   Initial group size: 1

**Additional workers** (can be started at any time using the join method):

.. code-block:: console

   $ ./join_server coordinator_address
   Joined group with 2 members

As you start more workers, they automatically join the group and are discovered
by all members.

Performance considerations
--------------------------

The centralized backend has some overhead:

**Network traffic**:

- Heartbeats: O(N) messages per heartbeat interval (N = group size)
- Joins: O(N) messages to notify all members
- View queries: 1 RPC to coordinator

**Coordinator load**:

- Must process heartbeats from all members
- Must track failure timeouts
- Must coordinate view updates

For large groups (>100 members), consider:

- Increasing heartbeat interval to reduce traffic
- Using hierarchical coordination (future feature)
- Partitioning into multiple smaller groups

Next steps
----------

- :doc:`09_client_api`: Learn about querying group membership from clients
- :doc:`10_group_view`: Detailed guide to working with group views
- :doc:`05_bootstrap_join`: Review joining groups with the centralized backend
