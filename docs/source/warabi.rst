Warabi (Blob storage component)
=================================

Warabi is a Mochi microservice that provides blob (Binary Large OBject) storage
capabilities for distributed HPC services. It is built on Thallium and offers multiple
storage backends including memory, persistent memory (pmem), and ABT-IO.

Warabi has been designed for flexibility and performance. It supports:

- **Multiple backends**: memory, persistent memory, ABT-IO, and dummy (for testing)
- **Regions**: Logical containers for organizing blobs
- **Transfer managers**: Configurable strategies for data transfer
- **Migration**: Moving data between providers or storage backends
- **Async operations**: Non-blocking I/O for improved performance
- **Bedrock integration**: Easy deployment and configuration

This section will walk you through a series of tutorials on how to use Warabi.

.. toctree::
   :maxdepth: 1

   warabi/01_intro.rst
   warabi/02_basics.rst
   warabi/03_backends_memory.rst
   warabi/04_backends_pmem.rst
   warabi/05_backends_abtio.rst
   warabi/06_transfer_managers.rst
   warabi/07_regions.rst
   warabi/08_migration.rst
   warabi/09_async.rst
   warabi/10_bedrock.rst
   warabi/11_c_api.rst
   warabi/12_python.rst
