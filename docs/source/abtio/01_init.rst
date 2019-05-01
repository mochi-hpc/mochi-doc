Initializing ABT-IO
===================

The following code shows how to initialize and finalize ABT-IO.
ABT-IO must be initialized after Argobots and finalized before it.
The :code:`abt_io_init` function takes the number of ES to create
for handling I/O operations. Alternatively, the :code:`abt_io_init_pool`
function can be used to make ABT-IO use an Argobots pool that was
alread created.

.. literalinclude:: ../../../code/abtio/01_init/main.c
       :language: cpp
