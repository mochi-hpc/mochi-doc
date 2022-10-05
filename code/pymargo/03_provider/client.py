import sys
from pymargo.core import Engine, MargoException
import pymargo

if __name__ == "__main__":
    with Engine('tcp', mode=pymargo.client) as engine:
        hello = engine.register("my_service_say_hello")
        address = engine.lookup(sys.argv[1])
        try:
            response = hello.on(address, provider_id=42)("Matthieu", lastname="Dorier")
            assert response == "Good bye Matthieu"
        except MargoException as e:
            print(e)
        address.shutdown()
