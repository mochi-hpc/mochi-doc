Backend: static
================

The static backend is a simple, lightweight group management implementation for
groups with fixed membership. Once initialized, the group membership cannot change.

When to use
-----------

Use the static backend when:

- Group membership is known at initialization and won't change
- You're using MPI and all processes start together
- You want minimal overhead and no centralized coordination
- You don't need fault tolerance or dynamic membership

Characteristics
---------------

**Fixed membership**: Once the group is initialized, members cannot join or leave.

**No coordination**: There is no centralized coordinator. Each member has a local
copy of the group view.

**Low overhead**: Minimal resource usage since there's no dynamic management.

**Deterministic**: Group membership is always the same across all members.

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
                   "bootstrap": "mpi",
                   "group": {
                       "type": "static",
                       "config": {}
                   },
                   "file": "mygroup.flock"
               }
           }
       ]
   }

The static backend takes an empty configuration object :code:`{}` as it has no
configurable options.

In C code
---------

.. literalinclude:: ../../../code/flock/07_backends_static/server.c
   :language: c

How it works
------------

With the static backend:

1. The group view is initialized once at provider creation
2. The view is stored locally in each member
3. No updates or synchronization occur after initialization
4. Queries return the static view

This means:

- :code:`flock_group_get_view` always returns the initial view
- There are no membership change notifications
- No network communication is needed for group queries

Best practices
--------------

**Use with appropriate bootstrap methods**:

- :code:`mpi`: All MPI ranks form a static group
- :code:`file`: Multiple processes load the same static view
- :code:`self`: Single-member group (can be useful for testing)

**Avoid with dynamic scenarios**:

- Don't use :code:`join` bootstrap with static backend (joining won't work)
- Don't expect fault tolerance (failed members stay in the view)
- Don't try to add/remove members programmatically

**File persistence**:

Always specify a :code:`file` option to persist the group view. This is an easy
way for client processes to lookup the view later.

Limitations
-----------

The static backend has several limitations:

1. **No dynamic membership**: Cannot add or remove members after initialization
2. **No fault tolerance**: Failed members remain in the view
3. **No membership updates**: View never changes
4. **Join not supported**: Processes can't join after initialization

If you need any of these capabilities, use the :doc:`08_backends_centralized` instead.

Performance
-----------

The static backend has excellent performance characteristics:

- **Zero runtime overhead**: No coordination or communication after initialization
- **Minimal memory**: Only stores the initial view
- **Instant queries**: No network calls needed

This makes it ideal for HPC applications where group membership is predetermined
and fixed.
