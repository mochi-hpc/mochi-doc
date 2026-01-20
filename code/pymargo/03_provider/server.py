from mochi.margo import Engine, Handle, provider, remote, on_finalize, on_prefinalize


@provider(service_name="my_service")
class MyProvider:

    @remote
    def say_hello(self, handle, firstname, lastname):
        print(f"Hello {firstname} {lastname}!")
        handle.respond(f"Good bye {firstname}")

    @on_finalize
    def finalize(self):
        print("Called when the engine finalizes")

    @on_prefinalize
    def prefinalize(self):
        print("Called right before the engine finalizes")


if __name__ == "__main__":
    with Engine("tcp") as engine:
        provider = MyProvider()
        engine.register_provider(provider, provider_id=42)
        print(f"Service running at {engine.address}")
        engine.enable_remote_shutdown()
        engine.wait_for_finalize()
