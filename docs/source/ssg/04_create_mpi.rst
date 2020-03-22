Creating a group from MPI
=========================

In the context of an MPI application, it is possible to
create an SSG group from an MPI comminucator using
the :code:`ssg_group_create_mpi` function as follows.

.. container:: toggle

    .. container:: header
    
       .. container:: btn btn-info

          main.c (show/hide)

    .. literalinclude:: ../../../code/ssg/04_create_mpi/main.c
       :language: cpp

MPI will be used for processes to exchange their address.
MPI will *not* be used for subsequent communications in SSG.
