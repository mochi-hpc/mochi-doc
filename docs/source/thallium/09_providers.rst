Working in terms of providers
=============================

It is often desirable for RPC to target a specific instance
of a class on the server side. Classes that can accept RPC
requests are called *providers*. A provider object is characterized 
by a provider id (of type :code:`uint16_t`). You will need to make
sure that no two providers use the same provider id. If they do,
they must expose RPC methods with different names
(e.g., providers of different services can have the same
provider id since they typically don't expose the same RPC names).

Server
------

The following code sample illustrates a custom 
provider class, :code:`my_sum_provider`, which exposes a number
of its methods as RPC.

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>
   #include <thallium/serialization/stl/string.hpp>

   namespace tl = thallium;

   class my_sum_provider : public tl::provider<my_sum_provider> {

       private:

       void prod(const tl::request& req, int x, int y) {
           std::cout << "Computing " << x << "*" << y << std::endl;
           req.respond(x+y);
       }

       int sum(int x, int y) {
           std::cout << "Computing " << x << "+" << y << std::endl;
           return x+y;
       }

       void hello(const std::string& name) {
           std::cout << "Hello, " << name << std::endl;
       }

       int print(const std::string& word) {
           std::cout << "Printing " << word << std::endl;
           return word.size();
       }

       public:

       my_sum_provider(tl::engine& e, uint16_t provider_id=1)
       : tl::provider<my_sum_provider>(e, provider_id) {
           define("prod", &my_sum_provider::prod);
           define("sum", &my_sum_provider::sum);
           define("hello", &my_sum_provider::hello);
           define("print", &my_sum_provider::print, tl::ignore_return_value());
       }

       ~my_sum_provider() {
           wait_for_finalize();
       }
   };

   int main(int argc, char** argv) {

       uint16_t provider_id = 22;
       tl::engine myEngine("tcp", THALLIUM_SERVER_MODE);
       std::cout << "Server running at address " << myEngine.self()
           << " with provider id " << provider_id << std::endl;
       my_sum_provider myProvider(myEngine, provider_id);

       return 0;
   }

This code defines the :code:`my_sum_provider` class, and creates
an instance of this class (passing the :code:`engine` as parameter
and a provider id). The :code:`my_sum_provider` class inherits
from :code:`thallium::provider<my_sum_provider>` to indicate that
this is a provider.

The RPC methods are exposed in the class constructor using the
:code:`define` method of the base provider class. Note that
multiple definitions of members are possible and exemplified here.

- "prod" is defined the same way as we previously defined RPCs using the engine, 
  with a function that returns :code:`void` and takes a :code:`const thallium::request&` as first parameter.
- "sum" is defined without the :code:`const thallium::request&` parameter. Since it returns an :code:`int`,
  Thallium will assume that this is what needs to be sent back to the client. It will therefore respond
  to the client with this return value.
- "hello" does not have a :code:`const thallium::request&` parameter either, and returns :code:`void`.
  Thallium will implicitly call :code:`.disable_response()` on this RPC to indicate that it does 
  not send a response back to the client.
- "print" does not have a :code:`const thallium::request&` parameter, and returns an :code:`int`.
  By default Thallium would consider that we want this return value to be sent to the client.
  To prevents this, we add the :code:`thallium::ignore_return_value()` argument, which indicates
  Thallium that the function should be treated as if it returned void.

Client
------

Let's now take a look at the client code.

.. code-block:: cpp

   #include <iostream>
   #include <thallium/serialization/stl/string.hpp>
   #include <thallium.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {
       if(argc != 3) {
           std::cerr << "Usage: " << argv[0] << " <address> <provider_id>" << std::endl;
           exit(0);
       }
       tl::engine myEngine("tcp", THALLIUM_CLIENT_MODE);
       tl::remote_procedure sum   = myEngine.define("sum");
       tl::remote_procedure prod  = myEngine.define("prod");
       tl::remote_procedure hello = myEngine.define("hello").disable_response();
       tl::remote_procedure print = myEngine.define("print").disable_response();
       tl::endpoint server = myEngine.lookup(argv[1]);
       uint16_t provider_id = atoi(argv[2]);
       tl::provider_handle ph(server, provider_id);
       int ret = sum.on(ph)(42,63);
       std::cout << "(sum) Server answered " << ret << std::endl;
       ret = prod.on(ph)(42,63);
       std::cout << "(prod) Server answered " << ret << std::endl;
       std::string name("Matthieu");
       hello.on(ph)(name);
       print.on(ph)(name);
   
       return 0;
   }

This client takes a provider id in addition to the server's address.
It uses it to define a :code:`thallium::provider_handle` object encapsulating
the server address and the provider id. This provider handle is then used in
place of the usual :code:`thallium::endpoint` to send RPCs to a specific
instance of provider.

.. important:: 
   We have called :code:`disable_response()` on the "hello" RPC
   here because there is no way for Thallium to infer here that this RPC
   does not send a response.
