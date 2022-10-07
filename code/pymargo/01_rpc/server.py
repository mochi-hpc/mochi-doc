from pymargo.core import Engine, Handle

def hello(handle: Handle, firstname: str, lastname: str):
    print(f"Hello {firstname} {lastname}!")
    handle.respond(f"Good bye {firstname}")

if __name__ == "__main__":
    with Engine("tcp") as engine:
        engine.register("hello", hello)
        print(f"Service running at {engine.address}")
        engine.enable_remote_shutdown()
        engine.wait_for_finalize()
