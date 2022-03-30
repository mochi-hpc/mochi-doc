Using information from a group
==============================

Once a process is a member or an observer of a group,
a number of operations may be done with the group id.
The code bellow illustrates most of them.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          main.c (show/hide)

    .. literalinclude:: ../../../code/ssg/08_info/main.c
       :language: cpp

In a group, a member is uniquely identified by a member id.
The member id of a process can be obtained from this process
using :code:`ssg_get_self_id()`. A member id can be used to retrieve
the corresponding process' address using :code:`ssg_get_group_member_addr()`
Member ids of the members in a group are non-contiguous.
Hence, each process can be identified with a rank as well, from 0
to *group_size-1*. The rank of the calling process can be obtained
using :code:`ssg_get_group_self_rank()`, and functions are available
to get a member id from a rank and vis-versa.

.. important::
   While the member id of a process is unique and remains the same
   regardless of changes to the group, the rank assigned to a process
   may change over time as processes join and leave the group.
