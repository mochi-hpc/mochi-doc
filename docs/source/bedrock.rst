Bedrock
=======

Composing various Mochi components together into a daemon program running
the service can be done manually, by writing a C, C++, or Python code that
initializes various providers and resolves dependencies between providers.
This method also requires to write custom bootstrapping mechanisms and
custom ways of configuring the different components of the service.
Alternatively, the composition and configuration of components can be
delegated to Bedrock.

Bedrock is a bootstrapping and configuration system for Mochi components.
It comes in the form of a program that can be run alone or with an MPI
or a PMIx context. This program takes a JSON configuration file specifying
the various components to instantiate and their dependencies. Bedrock
also allows to retrieve the configuration of a service at any point during
its run time.

This section of the documentation goes through the use of Bedrock,
from deploying components, to writing Bedrock modules for your own components.


.. toctree::
   :maxdepth: 1

   bedrock/01_start.rst
   bedrock/02_json.rst
   bedrock/03_query.rst
   bedrock/04_module.rst
   bedrock/06_jx9.rst
