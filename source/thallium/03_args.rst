Sending arguments, returning values
===================================

In this example we will define a "sum" RPC that will take two integers
and return their sum.

Server
------

Here is the server code.

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>

   namespace tl = thallium;

   void sum(const tl::request& req, int x, int y) {
       std::cout << "Computing " << x << "+" << y << std::endl;
       req.respond(x+y);
   }

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp://127.0.0.1:1234", THALLIUM_SERVER_MODE);
       myEngine.define("sum", sum);

       return 0;
   }

Notice that our :code:`sum` function now takes two integers in addition
to the const reference to a :code:`thallium::request`. You can also see
that this request object is used to send a response back to the client.
Because the server now sends something back to the client, we do not call
:code:`ignore_response()` when defining the RPC.

Client
------

Let's now take a look at the client code.

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp", THALLIUM_CLIENT_MODE);
       tl::remote_procedure sum = myEngine.define("sum");
       tl::endpoint server = myEngine.lookup("tcp://127.0.0.1:1234");
       int ret = sum.on(server)(42,63);
       std::cout << "Server answered " << ret << std::endl;

       return 0;
   }

The client calls the remote procedure with two integers and gets an integer back.
This way of passing parameters and returning a value hides many implementation
details that are handled with a lot of template metaprogramming.
Effectively, what happens is the following.
When passing the :code:`sum` function to :code:`engine::define`, the compiler
deduces from its signature that clients will send two integers.
Thus it creates the code necessary to deserialize two integers
before calling the function.

On the client side, calling :code:`sum.on(server)(42,63)` makes the compiler
realize that the client wants to serialize two integers and send them
along with the RPC. It therefore also generates the code for that.
The same happens when calling :code:`req.respond(...)` in the server,
the compiler generates the code necessary to serialize whatever object has been passed.

Back on the client side, :code:`sum.on(server)(42,63)` does not actually return an integer.
It returns an instance of :code:`thallium::packed_response`, which can be cast into any type,
here an integer. Asking the :code:`packed_response` to be cast into an integer also instructs
the compiler to generate the right deserialization code.

.. warning::
   A common miskate consists of changing the arguments accepted by an RPC handler
   but forgetting to update the calls to that RPC on clients. This can lead to data
   corruptions or crashes. Indeed, Thallium has no way to check that the types passed
   by the client to the RPC call are the ones expected by the server.

.. warning::
   Another common mistake is to use integers of different size on client and server.
   For example :code:`sum.on(server)(42,63);` on the client side will serialize two
   int values, because int is the default for integer litterals. If the corresponding
   RPC handler on the server side had been
   :code:`void sum(const tl::request& req, int64_t x, int64_t y)`,
   the call would have led to data corruptions and potential crash. One way to ensure
   that the right types are used is to explicitely cast the litterals:
   :code:`sum.on(server)(static_cast<int64_t>(42), static_cast<int64_t>(63));`.
