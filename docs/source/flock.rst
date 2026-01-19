Flock (Group management service)
=================================

Flock is a Mochi microservice that provides group management capabilities for
distributed services. It enables multiple processes to form groups, discover each
other, and coordinate membership. Flock is based on Margo and provides multiple
bootstrap methods and backends to suit different deployment scenarios.

Flock has been designed to support dynamic group membership, allowing processes
to join and leave groups, and to handle failures gracefully. It supports MPI,
file-based, and network-based bootstrapping methods.

This section will walk you through a series of tutorials on how to use Flock.

.. toctree::
   :maxdepth: 1

   flock/01_intro.rst
   flock/02_bootstrap_self.rst
   flock/03_bootstrap_view.rst
   flock/04_bootstrap_mpi.rst
   flock/05_bootstrap_join.rst
   flock/06_bootstrap_file.rst
   flock/07_backends_static.rst
   flock/08_backends_centralized.rst
   flock/09_client_api.rst
   flock/10_group_view.rst
   flock/11_bedrock.rst
   flock/12_cpp.rst
   flock/13_python.rst
