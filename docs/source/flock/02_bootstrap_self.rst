Bootstrap method: self
======================

The "self" bootstrap method is the simplest way to initialize a Flock group.
It creates a single-member group containing only the current process/provider.

When to use
-----------

Use the "self" bootstrap method when:

- You're starting with a single process and will dynamically add members later
- You're testing or prototyping
- Your service doesn't need initial coordination with other processes

Configuration
-------------

In Bedrock configuration:

.. literalinclude:: ../../../code/flock/01_intro/bedrock-config.json
   :language: json

In C code
---------

When creating a provider programmatically, use :code:`flock_group_view_init_from_self`:

.. literalinclude:: ../../../code/flock/02_bootstrap_self/server.c
   :language: c

The :code:`flock_group_view_init_from_self` function takes:

- The Margo instance
- The provider ID
- A pointer to the group view to initialize

This will populate the view with a single member (the current process).

Group view structure
--------------------

After initialization with "self", the group view contains:

- A single member with the current process's address and provider ID
- Empty metadata
- An initial digest for view versioning

You can persist this view to a file or share it with other processes to allow
them to join the group using the "join" or "view" bootstrap methods.
