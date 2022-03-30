Joining and leaving a group
===========================

In the following example, Process 1 bellow creates a group
with only itself as member, then puts the main ULT to sleep
for 10 seconds before destroying it and terminating.
Mercury communication proceed in the backgroup to detect
joining and leaving processes.

The process uses :code:`ssg_group_id_store()` to store information
about the group in a file. The first argument of this function is
the file name. The second is the group id. The third is the number
of current member addresses to include in the file.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          proc1.c (show/hide)

    .. literalinclude:: ../../../code/ssg/06_join_leave/proc1.c
       :language: cpp


Process 2, whose code is shown bellow, reads the group file using
:code:`ssg_group_id_load()`. It then joins the group by calling
:code:`ssg_group_join()`. Note that this function takes a membership
callback and user-data pointer, but does not include the group name
nor the group configuration. These pieces of information are retrieved
from the loaded group itself.

The process sleeps for 2 seconds, then calls :code:`ssg_group_leave()`
to leave the group. When running this code, you should see Process 1 display
messages upon Process 2 joining and leaving.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          proc2.c (show/hide)

    .. literalinclude:: ../../../code/ssg/06_join_leave/proc2.c
       :language: cpp

Note that :code:`ssg_group_join` and :code:`ssg_group_leave` both
have a variant, :code:`ssg_group_join_target` and :code:`ssg_group_leave_target`,
respectively, which can be used to specify he address of another member of
the group to inform of the process joining/leaving. The
:code:`ssg_group_join` and :code:`ssg_group_leave` functions simply
contact a random group member for this purpose.
