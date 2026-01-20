.. Mochi documentation master file, created by
   sphinx-quickstart on Thu Apr 11 10:32:12 2019.

Welcome to the Mochi project
============================


The `Mochi project <https://www.mcs.anl.gov/research/projects/mochi>`_ is a
collaboration between Argonne National Laboratory,
Los Alamos National Laboratory, Carnegie Mellon University, and the HDF Group.
The objective of this project is to explore a software defined storage approach
for composing storage services that provides new levels of functionality,
performance, and reliability for science applications at extreme scale.

This website gathers documentation and tutorials for the main libraries
used or developed in the context of the Mochi project.

Getting started
---------------

There are multiple ways of getting started with Mochi depending on your goal.
In all cases though, we recommend reading about Margo first. Margo is what binds
Mercury (RPC/RDMA) and Argobots (threading/tasking) under a runtime that all
the other components will use. If you are into C++, you may then want to read
about Thallium, which is a C++ library written on top of Margo.

If you want to use some of our components, you can then jump to their corresponding
tutorial sections. If you want to *develop* your own component, your can jump
to the Mochi templates and to the Bedrock bootstrapping system.


The core Mochi libraries
------------------------

**Margo** is a C library enabling the development of distributed HPC services.
It relies on Mercury for RPC/RDMA, and Argobots for threading/tasking, hidding
the complexity of these two libraries under a simple programming model.

**Thallium** is a C++ library wrapping Margo and enabling the development of
the same sort of services using all the power of modern C++. It is the
recommended library for C++ developers. Note that Thallium also provides
C++ wrappers to Argobots.

**Mercury** is Mochi's underlying RPC/RDMA library. While it is not necessary
to understand how to use Mercury itself when developing with Margo or Thallium
(which we recommend), we provide a set of tutorials for those who would need
to use it directly rather than through higher level libraries.

**Argobots** is used for threading/tasking in Mochi. Understanding its underlying
programming model may not be necessary at first, for simple Margo or Thallium
services, but may become useful to optimize performance or customize the
scheduling and placement of threads and tasks in a Mochi service.

**PyMargo** is a Python binding for the Margo library, allowing the development
of Mochi services in Python.

.. note::
   The tutorials for each of these libraries are mostly independent of one another.
   Feel free to start with the most relevant for you and jump from a library to another
   as you need.

.. important::
   In all the tutorials, we use the term "server" to denote a process to which
   one can send RPC requests, and "client" to denote a process that sends such
   RPC requests. It is important to note however that a server can also send
   RPC requests to other servers, and even to itself.

Other Mochi libraries/components
--------------------------------

**Yokan** is Mochi's main key/value storage service. It provides many
backends and a rich API, including C, C++, and Python bindings.

**Warabi** is Mochi's blob storage service. It provides capabilities for
storing binary large objects with support for multiple backends (memory,
persistent memory, ABT-IO) and efficient bulk transfers.

**Flock** is Mochi's group management service. It provides functionalities
to form and manage groups of distributed processes with multiple bootstrap
methods (self, view, MPI, join, file) and backends (static, centralized).

**ABT-IO** is a small library that can be used to offload POSIX
I/O operations to dedicated execution stream to better integrate with
the core Mochi libraries. ABT-IO depends on Argobots only.

Developing and deploying a service
----------------------------------

**Bedrock** is a bootstrapping system for composed Mochi services.
It provides a simple a unified way of deploying Mochi components in a process
and configure them using a JSON file. It also enable querying and changing
this configuration at run time.

**Templates** are provided to help users develop their own components
using Margo or Thallium. These templates can save a tremendous amount
of time and will let you focus on the important part of your service: its
RPCs and its API.

Contents
========

.. toctree::
   :maxdepth: 2

   installing.rst
   hello-mochi.rst
   margo.rst
   thallium.rst
   mercury.rst
   argobots.rst
   pymargo.rst
   yokan.rst
   warabi.rst
   flock.rst
   abtio.rst
   bedrock.rst
   templates.rst
   interop.rst
   misc.rst
   tutorials.rst


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
