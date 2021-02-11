Starting with Bedrock
=====================

In this tutorial, we will install Bedrock and deploy a simple (empty)
Bedrock-based service.

Installing Bedrock
------------------

Bedrock can be installed with Spack using the following command.

.. code-block:: console

   spack install mochi-bedrock

Starting a Bedrock process
--------------------------

Once installed a Bedrock process can be started as follows.

.. code-block:: console

   bedrock <protocol>

Where *<protocol>* is the protocol to use, for instance *na+sm*.
This command starts an "empty" Bedrock process, in the sense that
we haven't asked it to start any component apart from a Margo
instance using the specified protocol.

This command can take additional parameters.

- *-c/--config <config.json>*: specifies the JSON configuration file.
- *-v/--verbose <level>*: logging level (*trace*, *debug*, *info*, *warning*, *error*, or *critical*).
- *--stdin*: pass the JSON configuration via stdin instead of *-c/--config*.

The next section will disect the content of a JSON configuration file.
