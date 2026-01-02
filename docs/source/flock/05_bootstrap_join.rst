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

To join a group programmatically:

.. code-block:: c

   #include <flock/flock-server.h>
   #include <flock/flock-bootstrap.h>

   // ... margo initialization ...

   struct flock_provider_args args = FLOCK_PROVIDER_ARGS_INIT;
   flock_group_view_t initial_view = FLOCK_GROUP_VIEW_INITIALIZER;
   args.initial_view = &initial_view;

   // Contact a group member to join
   const char* member_addr = "na+sm://12345-0";
   uint16_t member_provider_id = 42;

   flock_group_view_init_from_join(mid, provider_id, member_addr,
                                     member_provider_id, &initial_view);

   // Use centralized backend for dynamic membership
   const char* config = "{ \"group\":{ \"type\":\"centralized\", \"config\":{} } }";
   flock_provider_register(mid, provider_id, config, &args, FLOCK_PROVIDER_IGNORE);

The :code:`flock_group_view_init_from_join` function takes:

- The Margo instance
- The provider ID of the new member
- The address of an existing member to contact
- The provider ID of that existing member
- A pointer to the group view to initialize

This function will contact the specified member and retrieve the current group view,
then add itself to the group.

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

   $ bedrock na+sm -c initial-member.json
   [info] Bedrock daemon now running at na+sm://12345-0

Then, start a second member that joins:

.. code-block:: json

   {
       "providers": [
           {
               "type": "flock",
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

.. code-block:: console

   $ bedrock na+sm -c join-member.json
   [info] Joined group with 2 members

Error handling
--------------

The join operation can fail if:

- The specified member is unreachable
- The member is not part of a Flock group
- The provider ID doesn't exist
- The backend doesn't support dynamic membership

Always check the return value and handle errors appropriately:

.. code-block:: c

   flock_return_t ret = flock_group_view_init_from_join(
       mid, provider_id, member_addr, member_provider_id, &initial_view);

   if (ret != FLOCK_SUCCESS) {
       fprintf(stderr, "Failed to join group: error %d\n", ret);
       // Handle error
   }

Next steps
----------

- :doc:`06_bootstrap_file`: Learn about loading views from files
- :doc:`08_backends_centralized`: Learn about the centralized backend for dynamic groups
- :doc:`10_group_view`: Detailed guide to working with group views
