Bootstrap method: join
=======================

The "join" bootstrap method allows a process to join an existing Flock group by
contacting one of its members. This is useful for dynamically adding processes to
a group after it has been initialized.

When to use
-----------

Use the "join" bootstrap method when:

- You want to add processes to an existing group
- You're implementing elastic services that scale up
- You have a bootstrap node that other processes can contact
- You want processes to discover the full group by contacting any member

Prerequisites
-------------

To use the join bootstrap method, you need:

- An existing Flock group with at least one member
- The address and provider ID of at least one group member
- A backend that supports dynamic membership (use "centralized", not "static")

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
                   "join": {
                       "address": "na+sm://12345-0",
                       "provider_id": 42
                   },
                   "group": {
                       "type": "centralized",
                       "config": {}
                   }
               }
           }
       ]
   }

The "join" section specifies:

- :code:`address`: The address of a member to contact
- :code:`provider_id`: The provider ID of that member

In C code
---------

To join a group programmatically, you use the client API to get the current
view from an existing group member, then add yourself to it:

.. literalinclude:: ../../../code/flock/05_bootstrap_join/server.c
   :language: c

The join process works as follows:

1. Create a Flock client to communicate with the existing group
2. Lookup the bootstrap server address
3. Create a group handle and get the current view
4. Add yourself to the view
5. Register the provider with the updated view

How it works
------------

The join process:

1. The new process contacts an existing group member via RPC
2. The member returns the current group view
3. The new process initializes with this view
4. If using a dynamic backend (centralized), the new member is added to the group
5. Other members are notified of the new member (depending on backend)

Backend considerations
----------------------

The join bootstrap method works differently depending on the backend:

**Static backend**:

- The new member can retrieve the group view
- But it cannot actually join (membership is fixed)
- Use this only for read-only access to the group

**Centralized backend**:

- The new member retrieves the view AND joins the group
- The centralized coordinator is notified
- Other members will learn about the new member
- This is the recommended backend for join

Example: Joining a running group
---------------------------------

First, start an initial member:

.. code-block:: console

   $ ./server self
   Server running at address na+sm://12345-0
   Bootstrapped as initial member (self)
   Flock provider registered

Then, start a second member that joins:

.. code-block:: console

   $ ./server na+sm://12345-0
   Server running at address na+sm://12346-0
   Joined existing group via na+sm://12345-0
   Group size: 2
   Flock provider registered

Next steps
----------

- :doc:`06_bootstrap_file`: Learn about loading views from files
- :doc:`08_backends_centralized`: Learn about the centralized backend for dynamic groups
- :doc:`10_group_view`: Detailed guide to working with group views
