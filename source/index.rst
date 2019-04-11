.. Mochi documentation master file, created by
   sphinx-quickstart on Thu Apr 11 10:32:12 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the Mochi project
============================


The Mochi project is a collaboration between Argonne National Laboratory,
Los Alamos National Laboratory, Carnegie Mellon University, and the HDF Group.
The objective of this project is to explore a software defined storage approach
for composing storage services that provides new levels of functionality,
performance, and reliability for science applications at extreme scale.

This website gathers documentation and tutorials for the main libraries 
used or developed in the context of the Mochi project:
Margo, Thallium, Argobots, and Mercury.

Margo is a C library enabling the development of distributed HPC services.
It relies on Mercury for RPC/RDMA, and Argobots for threading/tasking, hidding
the complexity of these two libraries under a simple programming model.

Thallium is a C++14 library wrapping Margo and enabling the development of
the same sort of services using all the power of modern C++. It is the
recommended library for C++ developers. Note that Thallium also provides
C++ wrappers to Argobots.

Argobots is used for threading/tasking in Mochi. Understanding its underlying
programming model may not be necessary at first, for simple Margo or Thallium
services, but may become useful to optimize performance or customize the
scheduling and placement of threads and tasks in a Mochi service.

Mercury is Mochi's underlying RPC/RDMA library. While it is not necessary
to undertand how to use Mercury itself when developing with Margo or Thallium
(which we recommend), we provide a set of tutorials for those who would need
to use it directly rather than through higher level libraries.

Note that documentation and tutorials on each of these libraries are
independent of one another. The reader is free to start with either library.

Contents
========

.. toctree::
   :maxdepth: 2

   installing.rst
   margo.rst
   thallium.rst
   argobots.rst
   mercury.rst


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
