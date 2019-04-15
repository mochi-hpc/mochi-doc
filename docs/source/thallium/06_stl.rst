Sending and returning STL containers
====================================

So far we have passed integers as arguments to the RPC handler
and returned integers as well. Note that Thallium is able to understand
and serialize all arithmetic types, that is, all integer types
(char, long, uint64_t, etc.) and floating point types (float, double, etc.).
Provided that the type(s) they contain are serializable, Thallium is also
capable of serializing all containers of the C++14 Standard Template Library.

List of containers
------------------

Here is a full list of containers that Thallium can serialize.

- :code:`std::array<T>`
- :code:`std::complex<T>`
- :code:`std::deque<T>`
- :code:`std::forward_list<T>`
- :code:`std::list<T>`
- :code:`std::map<K,V>`
- :code:`std::multimap<K,V>`
- :code:`std::multiset<T>`
- :code:`std::pair<U,V>`
- :code:`std::set<T>`
- :code:`std::string`
- :code:`std::tuple<T...>`
- :code:`std::unordered_map<K,V>`
- :code:`std::unordered_multimap<K,V>`
- :code:`std::unordered_multiset<T>`
- :code:`std::unordered_set<T>`
- :code:`std::vector<T>`

For instance, Thallium will be able to serialize the following type:

:code:`std::vector<std:tuple<std::pair<int,double>,std::list<int>>>`

Indeed, Thallium knows how to serialize ints and doubles, so it knows
how to serialize :code:`std::pair<int,double>` and :code:`std::list<int>`,
so it knows how to serialize :code:`std:tuple<std::pair<int,double>,std::list<int>>`,
so it knows how to serialize :code:`std::vector<std:tuple<std::pair<int,double>,std::list<int>>>`.

In order for Thallium to know how to serialize a given type,
however, you need to include the proper header in the files containing
code requiring serialization. For instance to make Thallium understand
how to serialize an :code:`std::vector`, you need :code:`#include <thallium/serialization/stl/vector.hpp>`.

The following is a revisited Hello World example in which the client sends its name as an :code:`std::string`.

Server
------

.. code-block:: cpp

   #include <string>
   #include <iostream>
   #include <thallium.hpp>
   #include <thallium/serialization/stl/string.hpp>

   namespace tl = thallium;

   void hello(const tl::request& req, const std::string& name) {
       std::cout << "Hello " << name << std::endl;
   }

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp://127.0.0.1:1234", THALLIUM_SERVER_MODE);
       myEngine.define("hello", hello).disable_response();

       return 0;
   }

Client
------

.. code-block:: cpp

   #include <string>
   #include <thallium.hpp>
   #include <thallium/serialization/stl/string.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp", THALLIUM_CLIENT_MODE);
       tl::remote_procedure hello = myEngine.define("hello").disable_response();
       tl::endpoint server = myEngine.lookup("tcp://127.0.0.1:1234");
       std::string name = "Matthieu";
       hello.on(server)(name);

       return 0;
   }

.. note:: 
   We explicitly define :code:`std::string name = "Matthieu";` 
   before passing it as an argument. If we were to write :code:`hello.on(server)("Matthieu");`,
   the compiler would consider :code:`"Matthieu"` as a :code:`const char*` variable, not a :code:`std::string`,
   and Thallium is not able to serialize pointers. Alternatively, :code:`hello.on(server)(std::string("Matthieu"));`
   is valid.
