import sys
from pymargo.core import Engine
import pymargo

if __name__ == "__main__":
    with Engine('tcp', mode=pymargo.client) as engine:
        do_bulk_transfer = engine.register("do_bulk_transfer")
        address = engine.lookup(sys.argv[1])
        buffer = b"This is a bytes buffer"
        local_bulk = engine.create_bulk(buffer, pymargo.bulk.read_only)
        req = do_bulk_transfer.on(address)(local_bulk, len(buffer), blocking=False)
        # do something in parallel
        response = req.wait() # wait for the request to complete
        assert response == "RDMA completed"
        address.shutdown()
