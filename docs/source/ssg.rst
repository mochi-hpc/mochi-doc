SSG (Group membership - deprecated)
===================================

SSG is a C library that manages groups of processes.
It can be used to monitor group membership for the purport of fault
tolerance or dynamic scaling, and to bootstrap a group from other
systems like MPI and PMIx.
SSG internally uses the SWIM protocol to monitor processes, detect
faults, and keep an eventually consistant view of the group using
a gossip protocol.

.. toctree::
   :maxdepth: 1

   ssg/01_init.rst
   ssg/02_create.rst
   ssg/03_create_file.rst
   ssg/04_create_mpi.rst
   ssg/05_create_pmix.rst
   ssg/06_join_leave.rst
   ssg/07_observe.rst
   ssg/08_info.rst
   ssg/09_serialize.rst
