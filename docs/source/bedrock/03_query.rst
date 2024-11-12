Querying a Bedrock configuration
================================

Once one or multiple Bedrock daemons have been deployed,
it is possible to query their complete configuration, either
via command line, or using a C++ program.

bedrock-query
-------------

:code:`bedrock-query` is a program installed with Bedrock
that can query the configuration of one or multiple daemons
and print it on its standard output. It is called as follows.

.. code-block:: console

   bedrock-query <protocol> -a <address>

*<protocol>* is the protocol used by the Bedrock daemon
(and consequently by the query program), e.g. :code:`na+sm`.
*<address>* is the Mercury address of the Bedrock daemon process.

The configuration is printed to the standard output in the form
of a JSON object associating the address with its configuration.

Multiple addresses can be passed, e.g.:

.. code-block:: console

   bedrock-query <protocol> -a <address1> -a <address2> -a <address3>

In this case, the resulting JSON object will have three entries,
one for each address. If all the daemons are gathered in an Flock group
represented by a group file of a given *<filename>*, the following command
will query all the members of the group.

.. code-block:: console

   bedrock-query <protocol> -f <filename>


The :code:`-i/--provider-id` flag may be used to specify a provider
id other than 0. For instance:

.. code-block:: console

   bedrock-query <protocol> -a <address> -i 42


Using Jx9 queries
-----------------

Sometimes it may be useful to have Bedrock daemons execute some code
to return only some specific information from their configuration.
This can be done using the `jx9 language <https://jx9.symisc.net/jx9_lang.html>`_.
From within a Jx9 script, the :code:`$__config__` variable contains
the configuration of the Bedrock daemon. The client will be sent back
the value returned by the script via its :code:`return` statement.

For example, the following Jx9 script makes the daemon return only
the names of the providers it manages.

.. literalinclude:: ../../../code/bedrock/03_query/query.jx9
   :language: php

Calling :code:`bedrock-query` with the :code:`-j <script>` argument
will read the Jx9 code from the provided script and send it to the
server for execution. For exemple:

.. code-block:: console

   bedrock-query <protocol> -a <address> -j query.jx9


Using a C++ program
-------------------

The easiest way to learn how to do all of the above in C++
is to look at how :code:`bedrock-query` is implemented, as well
as the *Client.hpp* and *ServiceHandle.hpp* files in *include/bedrock*.
