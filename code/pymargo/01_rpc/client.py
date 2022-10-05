import sys
from pymargo.core import Engine
import pymargo

if __name__ == "__main__":
    with Engine('tcp', mode=pymargo.client) as engine:
        hello = engine.register("hello")
        address = engine.lookup(sys.argv[1])
        response = hello.on(address)("Matthieu", lastname="Dorier")
        assert response == "Good bye Matthieu"
        address.shutdown()
