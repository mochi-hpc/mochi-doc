Bootstrap method: view
=======================

The "view" bootstrap method allows you to initialize a Flock group from a pre-constructed
group view. This is useful when you have a known set of members that have coordinated
between themselves ahead of initializing flock, and want to initialize
their flock provider with the same view of the group.

When to use
-----------

Use the "view" bootstrap method when:

- You have a predetermined list of group members, and you already know their address
- You want to programmatically construct a group view
- You're initializing multiple processes with a shared view
- You need fine-grained control over the initial group membership

Configuration
-------------

In a Bedrock configuration, you can't directly specify the view bootstrap method
because views must be constructed programmatically.

In C code
---------

To use the view bootstrap method programmatically, you can load a view from a file:
Bootstrapping from a view assumes that you have a way of creating a consistent view
across members beforehand. Here is an example where such a view is provided in a JSON
file, read using ``flock_group_view_init_from_file``.

.. literalinclude:: ../../../code/flock/03_bootstrap_view/server.c
   :language: c

Sharing views between processes
--------------------------------

Once you have a group view, you can:

1. **Serialize to JSON**: Convert the view to JSON format for easy sharing
2. **Write to file**: Save the view to a file that other processes can read
3. **Send via RPC**: Transmit the view to other processes using Mercury RPC

Other processes can then use the "file" bootstrap method to initialize from this view.

View operations
---------------

The group view API provides several operations:

- :code:`flock_group_view_add_member`: Add a member to the view
- :code:`flock_group_view_add_metadata`: Add metadata key-value pair
- :code:`flock_group_view_serialize`: Convert view to JSON string
- :code:`flock_group_view_deserialize`: Parse view from JSON string
- :code:`flock_group_view_clear`: Free view resources
- :code:`flock_group_view_copy`: Create a copy of a view
