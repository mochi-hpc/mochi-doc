Creating a group
================

The sample code hereafter shows how to create an SSG group.
The :code:`ssg_group_create()` function requires a margo instance id,
a group name, an array of null-terminated strings representing the list
of addresses of processes that are members of this group, the number of
addresses in this array, a configuration structure, a membership update
callback, and a pointer to user data for this callback.
The last argument is the resulting opaque group id.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          main.c (show/hide)

    .. literalinclude:: ../../../code/ssg/02_create/main.c
       :language: cpp

In this example we initialize a group of only one process.
When multiple processes create a group by this way, all the members of
the group have to provide the same input parameters (group name, array
of addresses, and configuration).

.. important::
   Because SSG group members have to send messages to each other, they
   need to be initialized as Margo servers and have an actively running
   process loop. Anything preventing the progress loop from running will
   prevent the process from responding in the SWIM protocol, which may lead
   to the process being marked as dead by other processes.

Group configuration
-------------------

The group configuration structure :code:`ssg_group_config_t` includes
the following parameters.
* :code:`swim_period_length_ms`: the number of milliseconds between each invokation of the SWIM protocol.
* :code:`swim_suspect_timeout_periods`: the number of periods of the SWIM protocol that should pass without a process answering for this process to be marked as *suspected*.
* :code:`swim_subgroup_member_count`: when a process A cannot reach a process B directly during the execution of the SWIM protocol, it will ask *swim_subgroup_member_count* to try reaching it on its behalf before considering it suspected.
* :code:`swim_disabled`: can be set to 1 to disable the SWIM protocol.
* :code:`ssg_credential`: some credential information.

Group membership callback
-------------------------

The :code:`my_membership_update_cb()` function will be called whenever a membership change is detected.
This membership change is indicated by the :code:`ssg_member_update_type_t` argument,
and the :code:`ssg_member_id_t` argument indicates which member joined, left, or died.

Leaving and destroying a group
------------------------------

The :code:`ssg_group_leave()` function is used to notify other members that the caller is
leaving the group. The :code:`ssg_group_destroy()` is then used to destroy the internal
data structures associated with the group.

If a member calls :code:`ssg_group_destroy()` without having called :code:`ssg_group_leave()` first,
other member will eventually consider that it has died.

It is possible for a process to call :code:`ssg_group_leave()` and then continue to
use the group id to lookup member addresses, as a simple observer of the group. However SSG
does not currently provide a way to rejoin a group once a process has left it.
