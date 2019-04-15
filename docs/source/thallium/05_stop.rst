Properly stopping a Thallium server
===================================

Stopping a server is done by calling the :code:`engine::finalize()` function.
By passing the :code:`engine` object into the closure of a lambda, as a reference,
we can make this very easy. For example:

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp://127.0.0.1:1234", THALLIUM_SERVER_MODE);

       std::function<void(const tl::request&, int, int)> sum =
           [&myEngine](const tl::request& req, int x, int y) {
               std::cout << "Computing " << x << "+" << y << std::endl;
               req.respond(x+y);
               myEngine.finalize();
           };

       myEngine.define("sum", sum);

       return 0;
   }

In this version of the server, the server will shut down after the first RPC is received.
Note that the engine is captured by reference. Thallium engines are indeed non-copyable.
