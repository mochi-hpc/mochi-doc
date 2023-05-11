Hands-on initial setup
======================

Please begin by creating a subdirectory called :code:`mochi-tutorial` on
your machine and cloning the hands-on exercise repositories into it:

.. code-block:: console

   mkdir mochi-tutorial
   cd mochi-tutorial
   git clone https://github.com/mochi-hpc-experiments/margo-tutorial-exercises.git
   git clone https://github.com/mochi-hpc-experiments/thallium-tutorial-exercises.git
   cd ..

The next step is to set up a Mochi development environment.
The most straightforward way to do this is by creating a Docker container
using an image that is preconfigured for use with the Mochi tutorial hands-on
exercises. This method is documented as **Option 1** below. If you prefer to use
native environment, please skip to **Option 2**.

Option 1 (preferred): create a development environment using Docker
-------------------------------------------------------------------

You need to first have Docker installed on your machine;
please see the `Docker installation instructions for your platform <https://docs.docker.com/get-docker/>`_.
Once Docker is installed, you can use the following commands to download a preconfigured image:

.. code-block:: console

   docker pull carns/mochi-tutorial:latest
   docker tag carns/mochi-tutorial:latest mochi-tutorial

The following run command will instantiate a new container from the mochi-tutorial image.
The container will have its name set to "mt1" (short for "mochi tutorial 1").

.. note::

   The container is configured to run indefinitely in detached mode and allow
   login for the "mochi" user with no password.

The run command will also map the :code:`mochi-tutorial` directory created in
the previous section to the :code:`/home/mochi/mochi-tutorial` directory within
the container for convenience. This will allow you to edit the code with your
editor of choice outside the container, and jump into the container to build
the code and run it (you can also choose to edit the code directory inside
the container using Vim if you prefer).

.. code-block:: console

   docker run --detach --name mt1 --volume $(pwd)/mochi-tutorial:/home/mochi/mochi-tutorial mochi-tutorial

Once the container is running, you can open a shell on it using the following command.

.. code-block:: console

   docker exec -it mt1 /bin/bash

From the container's command prompt you should be in the :code:`/home/mochi` directory,
with subdirectories available for :code:`spack`, :code:`mochi-spack-packages`, and the
:code:`mochi-tutorial` directory that is mapped to your host machine and populated with
tutorial exercise files.

.. code-block:: console

   > docker exec -it mt1 /bin/bash

   mochi@d3c9c489a2c1:~$ ls
   mochi-spack-packages  mochi-tutorial  spack

   mochi@d3c9c489a2c1:~$ ls mochi-tutorial
   margo-tutorial-exercises

We recommand that you open multiple shells while following these exercises,
so that you can build the code, run a server and run a client in different terminals

You can use the following commands to stop and restart the "mt1" container.

.. code-block:: console

   docker stop mt1
   docker start mt1

If you need more detailed instructions or want to build the docker image yourself
from its Dockerfile recipe, please refer to
`these instructions <https://github.com/mochi-hpc-experiments/mochi-docker/>`_.


Option 2: create a development environment manually
---------------------------------------------------

.. important::

   Spack setup and administration is beyond the scope of this tutorial;
   please do not use this option unless you already have an existing Spack
   configuration that you are comfortable using.

If you have Spack already installed and setup on your machine, simply make
sure that you have the Mochi namespace available for Spack to use. This can be done as follows.

.. code-block:: console

   git clone https://github.com/mochi-hpc/mochi-spack-packages.git
   spack repo add mochi-spack-packages


What's next ?
-------------

You are now ready to carry on with either the Margo or the Thallium exercises.
Choose one depending on your preferred programming language (C or C++ respectively).
