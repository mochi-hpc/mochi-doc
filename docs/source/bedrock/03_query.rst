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
one for each address. If all the daemons are gathered in an SSG group
represented by a group file of a given *<filename>*, the following command
will query all the members of the group.

.. code-block:: console

   bedrock-query <protocol> -s <filename>


The :code:`-i/--provider-id` flag may be used to specify a provider
id other than 0. for instance:

.. code-block:: console

   bedrock-query <protocol> -a <address> -i 42


Using a C++ program
-------------------

The easiest way to learn how to do all of the above in C++
is to look at how :code:`bedrock-query` is implemented, as well
as the *Client.hpp* and *ServiceHandle.hpp* files in *include/bedrock*.
