More on admin and configuration
===============================

This tutorial provides more information on the admin interface and
on configuring the server and its databases.

The admin interface
-------------------

The admin library provides functions to *open*, *close*, *destroy*,
and *list* databases. This API is showcased in the code bellow.


.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          admin.c (show/hide)

    .. literalinclude:: ../../../code/yokan/02_admin/admin.c
       :language: cpp


Depending on the backend used, opening a database may create
files, or request to open existing files. Closing a database will
make it inaccessible to clients. If the database was in memory,
it will also erase its contents. If it was backed up by files, it
will close those files. Destroying the database will not only
close it, it will also erase all corresponding files from the
unerlying file system.

The code above also shows the use of *security tokens* ("ABCD").
This mechanism is a rudimentary security check. This string can
be used when initializing a provider so that operations from
the admin library need the same token to proceed. These token
are generally not needed in most use cases and can be set to NULL.

Creating databases without an admin
-----------------------------------

In the previous tutorial we have hand-written an admin program
to create a database. This is not needed, however, as we could have
put the database definition directly in the Bedrock configuration
file, as follows.

.. literalinclude:: ../../../code/yokan/02_admin/bedrock.json
   :language: json

By doing so, we see in Bedrock's standard output the database being
created, and we can grab its identifier.

.. code-block:: console

   $ bedrock na+sm -c config.json.
   [2021-10-14 12:16:24.608] [info] [yokan] opened database 0361eb8b-d739-4950-89a2-26f16ea3db
   [2021-10-14 12:16:24.608] [info] [yokan] YOKAN provider registration done
   [2021-10-14 12:16:24.608] [info] Bedrock daemon now running at na+sm://8972-0

Similarly, when embedding a provider into C code, we can pass a configuration
string that contains the definition of a database, as follows.

.. container:: toggle

    .. container:: header

       .. container:: btn btn-info

          server.c (show/hide)

    .. literalinclude:: ../../../code/yokan/02_admin/server.c
       :language: cpp
