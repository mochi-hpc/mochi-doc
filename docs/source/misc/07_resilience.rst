Designing resilient services
============================

If the receiver of an RPC crashes during the execution of said RPC,
the sender will end up waiting indefinitely for its response.
Timed RPC (:code:`margo_forward_timed`, :code:`thallium::callable_remote_procedure::timed`)
offers a way to cancel an RPC after a given amount of time. However, using them
can pose more problems, in particular if the receiver is alive, simply taking
too much time to complete, and has RDMA operations to carry out. These RDMA operations
may end up pulling from or pushing into stale memory on the sender's side.

A recommended pattern to circumvent this issue is to rely on the following pattern:

- Have the sender send the RPC with a timeout :code:`T`, and include this timeout as an RPC argument;
- Upon receiving an RPC, the receiver computes :code:`T' = T/2`, this is the receiver's time budget
  for completing the RPC;
- If the RPC includes RDMA operations, take :code:`T'` into consideration and issue a timed RDMA transfer.

For example, if the sender sends :code:`T = 10` milliseconds, the server computes :code:`T' = 5` milliseconds.
Assuming it then takes 1 millisecond to reach the RDMA transfer, the server can start the RDMA transfer with a
timeout of 4 milliseconds.

This method is not fool-proof, but can be a useful tool to develop more resilient data services.
Beyond this, techniques such as replication, consensus, two-phase commit, or rollback, may be used
as needed depending on each specific service.
