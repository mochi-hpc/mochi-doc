from pymargo.core import Engine, Handle
from pymargo.bulk import Bulk
import pymargo.bulk

class Receiver:

    def __init__(self, engine):
        self.engine = engine

    def do_bulk_transfer(self, handle: Handle, remote_bulk: Bulk, bulk_size: int):
        local_buffer = bytes(bulk_size)
        local_bulk = self.engine.create_bulk(local_buffer, pymargo.bulk.write_only)
        req = self.engine.transfer(
            op=pymargo.bulk.pull,
            origin_addr=handle.address,
            origin_handle=remote_bulk,
            origin_offset=0,
            local_handle=local_bulk,
            local_offset=0,
            size=bulk_size,
            blocking=False)
        # do something in parallel here
        req.test() # will return True if the request completed
        req.wait() # will block until request is completed
        # note: req.wait() is still necessary even if the req.test()
        # returned true, as it frees the internal request
        print(local_buffer)
        req = handle.respond("RDMA completed", blocking=False)
        # do more things will response is sent
        req.wait() # block until response is sent

if __name__ == "__main__":
    with Engine('tcp') as engine:
        receiver = Receiver(engine)
        engine.register("do_bulk_transfer", receiver.do_bulk_transfer)
        print(f"Service running at {engine.address}")
        engine.enable_remote_shutdown()
        engine.wait_for_finalize()
