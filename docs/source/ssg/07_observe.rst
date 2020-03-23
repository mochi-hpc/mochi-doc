Observing an SSG group
======================

It may be useful for a process to be aware of an SSG group
(i.e. be able to access the address of its members) without
being itself a member of the group. We call such a process an
*observer*.

In the code samples bellow, Process 1 creates a group and stays alive
for 10 seconds. Process 2 reads the group id from the file created
by Process 1, and start observing it using :code:`ssg_group_observe`.
It then stops observing it using :code:`ssg_group_unobserve`.
Note the use of :code:`ssg_group_destroy` instead of :code:`ssg_group_leave`
in the observer process.

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

TODO: what does this mean concretely to "observe a group"?
