Transferring data over RDMA
===========================

In this tutorial, we will learn how to transfer data over RDMA.
The class at the core of this tutorial is :code:`thallium::bulk`.
This object represents a series of segments of memory within the
current process or in a remote process, that is exposed for remote
memory accesses.

Client
------

Here is an example of a client sending a "do_rdma" RPC with a bulk object as argument.

.. code-block:: cpp

  #include <iostream>
  #include <thallium.hpp>

  namespace tl = thallium;

  int main(int argc, char** argv) {

      tl::engine myEngine("tcp", MARGO_CLIENT_MODE);
      tl::remote_procedure remote_do_rdma = myEngine.define("do_rdma").disable_response();
      tl::endpoint server_endpoint = myEngine.lookup("tcp://127.0.0.1:1234");

      std::string buffer = "Matthieu";
      std::vector<std::pair<void*,std::size_t>> segments(1);
      segments[0].first  = (void*)(&buffer[0]);
      segments[0].second = buffer.size()+1;

      tl::bulk myBulk = myEngine.expose(segments, tl::bulk_mode::read_only);

      remote_do_rdma.on(server_endpoint)(myBulk);

      return 0;
  }

In this client, we define a buffer with the content "Matthieu"
(because it's a string, there is actually a null-terminating character).
We then define segments as a vector of pairs of :code:`void*` and :code:`std::size_t`.
Each segment (here only one) is characterized by its starting address in local
memory and its size. We call :code:`engine::expose` to expose the buffer and
get a :code:`bulk` instance from it. We specify :code:`tl::bulk_mode::read_only`
to indicate that the memory will only be read by other processes
(alternatives are :code:`tl::bulk_mode::read_write` and :code:`tl::bulk_mode::write_only`).
Finally we send an RPC to the server, passing the bulk object as an argument.

Server
------

Here is the server code now:

.. code-block:: cpp

   #include <iostream>
   #include <thallium.hpp>
   #include <thallium/serialization/stl/string.hpp>

   namespace tl = thallium;

   int main(int argc, char** argv) {

       tl::engine myEngine("tcp://127.0.0.1:1234", THALLIUM_SERVER_MODE);

       std::function<void(const tl::request&, tl::bulk&)> f =
           [&myEngine](const tl::request& req, tl::bulk& b) {
               tl::endpoint ep = req.get_endpoint();
               std::vector<char> v(6);
               std::vector<std::pair<void*,std::size_t>> segments(1);
               segments[0].first  = (void*)(&v[0]);
               segments[0].second = v.size();
               tl::bulk local = myEngine.expose(segments, tl::bulk_mode::write_only);
               b.on(ep) >> local;
               std::cout << "Server received bulk: ";
               for(auto c : v) std::cout << c;
               std::cout << std::endl;
           };
       myEngine.define("do_rdma",f).ignore_response();
   }

In the RPC handler, we get the client's :code:`endpoint` using
:code:`req.get_endpoint()`. We then create a buffer of size 6.
We initialize :code:`segments` and expose the buffer to get a :code:`bulk`
object from it. The call to the :code:`>>` operator pulls data from
the remote :code:`bulk` object :code:`b` and the local :code:`bulk` object.
Since the local :code:`bulk` is smaller (6 bytes) than the remote one (9 bytes),
only 6 bytes are pulled. Hence the loop will print *Matthi*.
It is worth noting that an :code:`endpoint` is needed for Thallium to know
in which process to find the memory we are pulling. That's what :code:`bulk::on(endpoint)`
does.

Understanding local and remote bulk objects
-------------------------------------------

A :code:`bulk` object created using :code:`engine::expose` is local.
When such a :code:`bulk` object is sent to another process, it becomes remote.
Operations can only be done between a local :code:`bulk` object and a remote :code:`bulk`
object resolved with an endpoint, e.g.,

.. code-block:: cpp

   myRemoteBulk.on(myRemoteProcess) >> myLocalBulk;

or

.. code-block:: cpp

   myLocalBulk >> myRemoteBulk.on(myRemoteProcess);

The :code:`<<` operator is, of course, also available.

Transferring subsections of bulk objects
----------------------------------------

It is possible to select part of a bulk object to be transferred.
This is done as follows, for example.

.. code-block:: cpp

   myRemoteBulk(3,45).on(myRemoteProcess) >> myLocalBulk(13,45);

Here we are pulling 45 bytes of data from the remote :code:`bulk`
starting at offset 3 into the local :code:`bulk` starting at its
offset 13. We have specified 45 as the number of bytes to be
transferred. If the sizes had been different, the smallest
one would have been picked.
