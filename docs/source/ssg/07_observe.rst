Observing an SSG group
======================

It may be useful for a process to be aware of an SSG group
(i.e. be able to access the address of its members) without
being itself a member of the group. We call such a process an
*observer*.

Contrary to group members, which are continuously updated
of membership changes, observers must periodically poll the group
to refresh their view.

In the code samples bellow, Process 1 creates a group and stays alive
for 10 seconds. Process 2 reads the group id from the file created
by Process 1, and uses :code:`ssg_group_refresh` to request an up-to-date
view to a randomly-selected member.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          proc1.c (show/hide)

    .. literalinclude:: ../../../code/ssg/07_observe/proc1.c
       :language: cpp


.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          proc2.c (show/hide)

    .. literalinclude:: ../../../code/ssg/07_observe/proc2.c
       :language: cpp

.. note::
   If all the group members have changed between two calls to :code:`ssg_group_refresh`,
   the observer will obviously not be able to update its view. If this is a likely scenario,
   we recommand that group members periodically store the group's state into a file so
   that observers can reload a proper group state :code:`ssg_group_refresh` fails.
