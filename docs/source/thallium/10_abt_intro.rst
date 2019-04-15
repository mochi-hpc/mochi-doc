Introduction to Argobots wrappers
=================================

For convenience, Thallium provides C++ wrappers to all the
Argobots functions and structures. We strongly encourage
users to familiarize themselves with Argobots to understand
how to use execution streams, threads, tasks, pools, etc.

The following code initializes Argobots through Thallium.

.. code-block:: cpp

   #include <thallium.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {
       // the constructor of the tl::abt object initializes Argobots
       tl::abt a;
       // its destructor finalizes it
       return 0;
   }

.. note:: 
   If you are creating a :code:`thallium::engine`, this engine
   already initializes Argobots in its constructor and finalizes it in its destructor.
   The code above should therefore be used either if Thallium is used only for
   its Argobots wrappers (no RPC definition), or if there is a need to initialize
   Argobots *before* initializing the engine (for example when creating custom
   Argobots scheduler and pools that the engine will then use).
