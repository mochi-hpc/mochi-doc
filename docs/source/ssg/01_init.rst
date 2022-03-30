Initializing SSG
================

The following code sample shows how to initialize and finalize
SSG.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          main.c (show/hide)

    .. literalinclude:: ../../../code/ssg/01_init/main.c
       :language: cpp

.. important::
   Though the Margo instance is not provided to the :code:`ssg_init`
   function, this function still needs Margo to have been initialized.
