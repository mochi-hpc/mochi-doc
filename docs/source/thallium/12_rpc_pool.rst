Using Argobots pools with Thallium RPCs
=======================================

Thallium allows RPC handlers to be associated with a particular
Argobots pool, so that any incoming request for that RPC gets
dispatched to the specified pool.

Server
------

The following code exemplifies using a custom pool in a server.

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>

   namespace tl = thallium;

   void sum(const tl::request& req, int x, int y) {
       std::cout << "Computing " << x << "+" << y << std::endl;
       req.respond(x+y);
   }

   int main(int argc, char** argv) {

       tl::abt scope;

       tl::engine myEngine("tcp", THALLIUM_SERVER_MODE);

       std::vector<tl::managed<tl::xstream>> ess;
       tl::managed<tl::pool> myPool = tl::pool::create(tl::pool::access::spmc);
       for(int i=0; i < 4; i++) {
           tl::managed<tl::xstream> es
               = tl::xstream::create(tl::scheduler::predef::deflt, *myPool);
           ess.push_back(std::move(es));
       }

       std::cout << "Server running at address " << myEngine.self() << std::endl;
       myEngine.define("sum", sum, 1, *myPool);

       myEngine.wait_for_finalize();

       for(int i=0; i < 4; i++) {
           ess[i]->join();
       }

       return 0;
   }

We are explicitly calling :code:`wait_for_finalize()`
(which is normally called in the engine's destructor)
before joining the execution streams because we don't
want the primary ES to be blocking on the :code:`join()` calls.

We are also using a :code:`tl::abt` object to initialize
Argobots because this prevents the engine from taking
ownership of the Argobots environment and destroy it
on the :code:`wait_for_finalize()` call.

.. important:: 
   This feature requires to provide a non-zero provider
   id (passed to the define call) when defining the RPC
   (here 1). Hence you also need to use provider handles
   on clients, even if you do not define a provider class.

Client
------

Here is the corresponding client.

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {
       if(argc != 2) {
           std::cerr << "Usage: " << argv[0] << " <address>" << std::endl;
           exit(0);
       }
       tl::engine myEngine("tcp", THALLIUM_CLIENT_MODE);
       tl::remote_procedure sum = myEngine.define("sum");
       tl::endpoint server = myEngine.lookup(argv[1]);
       tl::provider_handle ph(server, 1);
       int ret = sum.on(ph)(42,63);
       std::cout << "Server answered " << ret << std::endl;

       return 0;
   }
