Using lambdas and objects to define RPC handlers
================================================

So far we have seen how to define an RPC handler using a function.
This method is not necessarily the best, since it forces the use of
global variables to access anything outside the function.

Fortunately, Thallium can use lambdas as RPC handlers, and even
any object that has parenthesis operator overloaded.
These objects and lambdas must be converted into :code:`std::function`
objects first. Here is how to rewrite the sum server using a lambda.

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp://127.0.0.1:1234", THALLIUM_SERVER_MODE);

       std::function<void(const tl::request&, int, int)> sum =
           [](const tl::request& req, int x, int y) {
               std::cout << "Computing " << x << "+" << y << std::endl;
               req.respond(x+y);
           };

       myEngine.define("sum", sum);

       return 0;
   }

The big advantage of lambdas is their ability to capture local variables,
which prevents the use of global variables to pass user-provided data into
RPC handlers. This will become handy in the next tutorial...
