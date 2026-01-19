Backend: centralized
====================

The centralized backend provides dynamic group management with a centralized
coordinator. It supports membership changes, fault detection (of processes other
than the coordinator), and notifications.

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

**Centralized coordination**: One member acts as the primary (coordinator), managing the
authoritative group view.

**Failure detection**: The primary periodically pings followers to detect failures.

**Notifications**: Members subscribe to membership change events. Clients can update
their view if the group changes.

**Higher overhead**: More resource usage than static backend due to coordination
(periodic pings).

Configuration
-------------

In Bedrock configuration:

.. literalinclude:: ../../../code/flock/11_bedrock/bedrock-config-centralized.json
   :language: json

Configuration options:

- :code:`ping_timeout_ms`: Timeout value when sending a ping RPC to a member
- :code:`ping_interval_ms`: Time to wait between two ping RPCs to the same member.
  Can be a single value or a tuple :code:`[min, max]` for a uniform random value in that intervals.
- :code:`ping_max_num_timeouts`: Number of consecutive ping timeouts before a member
  is considered dead and removed from the group
- :code:`primary_address`: (optional) Address of the process to use as primary (coordinator).
  If not provided, the first member in the initial view is used.
- :code:`primary_provider_id`: (optional) Provider ID of the primary process.

Example with randomized ping interval:

.. code-block:: json

   {
       "group": {
           "type": "centralized",
           "config": {
               "ping_timeout_ms": 2000,
               "ping_interval_ms": [500, 1500],
               "ping_max_num_timeouts": 3
           }
       }
   }

In C code
---------

.. literalinclude:: ../../../code/flock/08_backends_centralized/server.c
   :language: c

How it works
------------

The centralized backend operates as follows:

**Primary selection**: By default, the first member in the initial view becomes the
primary (coordinator). You can override this with :code:`primary_address` and
:code:`primary_provider_id` configuration options.

**Ping mechanism**: The primary periodically pings all followers to check if they
are still alive.

**Join protocol**:

1. New member contacts an existing member with "join" bootstrap
2. The request is forwarded to the primary member
3. Primary adds the new member to the view
4. All members are notified of the new member
5. New member receives the updated view

**Failure detection**:

1. Primary sends ping RPCs to followers at regular intervals
2. If a ping times out, the timeout counter for that member increments
3. After :code:`ping_max_num_timeouts` consecutive timeouts, the member is removed
4. All remaining members are notified of the change

Dynamic membership
------------------

Unlike the static backend, the centralized backend supports adding and removing members:

**Adding members**:

Use the "join" bootstrap method to add members to a running group.

**Removing members**:

Members are automatically removed when they:

- Fail to respond to pings (failure detection)
- Call :code:`flock_provider_deregister` (graceful shutdown)

Primary resilience
------------------

The primary is a single point of failure. If the primary fails:

- Followers will no longer receive pings or view updates
- The group becomes effectively frozen

Best practices:

- Run the primary on a reliable node
- Monitor the primary's health
- Consider restarting the group if the primary fails

Example: Elastic service
------------------------

Here's an example of building an elastic service that can scale up dynamically:

**Initial primary**:

.. code-block:: console

   $ ./server
   Flock provider registered with CENTRALIZED backend
   Group membership can change dynamically
   Initial group size: 1

**Additional workers** (can be started at any time using the join method):

.. code-block:: console

   $ ./join_server mygroup.flock
   Joined group with 2 members

As you start more workers, they automatically join the group and are discovered
by all members.

Performance considerations
--------------------------

The centralized backend has some overhead:

**Network traffic**:

- Pings: O(N) messages per ping interval (N = number of followers)
- Joins: O(N) messages to notify all members
- View queries: May require RPC depending on caching

**Primary load**:

- Must send pings to all followers
- Must track timeout counters for each follower
- Must coordinate view updates

For large groups (>100 members), consider:

- Increasing ping interval to reduce traffic
- Partitioning into multiple smaller groups

Tuning parameters
-----------------

Choose ping and timeout values based on your needs:

**Fast failure detection** (more overhead):

.. code-block:: json

   {
       "ping_timeout_ms": 1000,
       "ping_interval_ms": 500,
       "ping_max_num_timeouts": 2
   }

Failure detection time: ~1.5 seconds (500ms interval + 2 × 1000ms timeouts)

**Slower failure detection** (less overhead):

.. code-block:: json

   {
       "ping_timeout_ms": 5000,
       "ping_interval_ms": 3000,
       "ping_max_num_timeouts": 3
   }

Failure detection time: ~18 seconds (3 × 3000ms intervals + 3 × 5000ms timeouts)
