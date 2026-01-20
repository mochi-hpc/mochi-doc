Using Flock with Bedrock
=========================

Bedrock is Mochi's service bootstrapping framework. It provides a unified way to
configure and deploy Flock providers using JSON configuration files.

Bedrock configuration examples are provided throughout all the tutorials in this
section, hence this page only provides a couple of examples for reference.

Prerequisites
-------------

Install Flock with Bedrock support:

.. code-block:: console

   spack install mochi-flock +bedrock

Basic configuration
-------------------

A minimal Bedrock configuration for Flock with the static backend:

.. literalinclude:: ../../../code/flock/11_bedrock/bedrock-config-static.json
   :language: json

Configuration with the centralized backend:

.. literalinclude:: ../../../code/flock/11_bedrock/bedrock-config-centralized.json
   :language: json
