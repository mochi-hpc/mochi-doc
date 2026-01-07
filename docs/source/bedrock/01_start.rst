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
instance using the specified protocol. Hence the only thing we can
do for now is query for its internal configuration, or shut it down.

This command can take additional parameters.

- :code:`-c/--config <config.json>`: specifies the configuration file (JSON by default).
- :code:`-v/--verbose <level>`: logging level (*trace*, *debug*, *info*, *warning*, *error*, *critical*, or *off*).
- :code:`--stdin`: pass the JSON configuration via stdin instead of :code:`-c/--config`.
- :code:`--jx9`: interpret the configuration file as JX9 script.
- :code:`--toml`: interpret the configuration file as TOML.

The next section will disect the content of a JSON configuration file.
