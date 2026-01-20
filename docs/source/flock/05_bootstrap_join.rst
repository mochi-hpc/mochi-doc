Bootstrap method: join
=======================

The "join" bootstrap method allows a process to dynamically join an existing Flock group.
Unlike the "file" or "view" bootstrap methods which simply load a static view, "join"
actively contacts existing group members and requests to be added to the group.

When to use
-----------

Use the "join" bootstrap method when:

- You want to dynamically add processes to a running group
- You're implementing elastic services that scale up at runtime
- You need existing group members to be notified of new members
- You want the group view to be updated across all members

Prerequisites
-------------

To use the join bootstrap method, you need:

- An existing Flock group with at least one member
- A group view file containing addresses of current members
- A backend that supports dynamic membership (use "centralized", not "static")

How it works
------------

The join process:

1. The new process loads a view (e.g. from a file) containing addresses of existing group members
2. It registers a provider with :code:`"bootstrap": "join"` in the configuration
3. The provider contacts existing members and requests to join
4. The existing members add the new member to the group
5. All members receive an updated view with the new member

This is different from the "file" bootstrap, which simply loads a view that we
expect the process to already be part of.

Configuration
-------------

In Bedrock configuration:

.. code-block:: json

   {
       "libraries": [
           "libflock-bedrock-module.so"
       ],
       "providers": [
           {
               "type": "flock",
               "name": "my_flock_provider",
               "provider_id": 42,
               "config": {
                   "bootstrap": "join",
                   "file": "mygroup.flock",
                   "group": {
                       "type": "centralized",
                       "config": {}
                   }
               }
           }
       ]
   }

The configuration specifies:

- :code:`bootstrap`: Must be "join" to enable dynamic joining
- :code:`file`: Path to a file containing the current group view
- :code:`group.type`: Must support dynamic membership (e.g. not be "static")

In C code
---------

To join a group programmatically:

.. literalinclude:: ../../../code/flock/05_bootstrap_join/server.c
   :language: c

The key steps are:

1. Load the existing group view from a file using :code:`flock_group_view_init_from_file`
2. Set :code:`"bootstrap": "join"` in the configuration JSON
3. Call :code:`flock_provider_register` with the configuration and initial view

The provider will then contact the members listed in the view and request to join the group.

Example workflow
----------------

**Step 1**: Start the initial group member with "self" bootstrap:

.. code-block:: console

   $ ./initial_server
   Server running at address na+sm://12345-0
   Flock provider registered
   Group file written to: mygroup.flock

**Step 2**: A new process joins using the group file:

.. code-block:: console

   $ ./05_flock_server mygroup.flock
   Server running at address na+sm://12346-0
   Loaded group view from file: mygroup.flock
   Current group size: 1
   Successfully joined the group

After joining, all members will have an updated view containing both members.
