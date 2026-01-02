import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

class WarabiTarget:
    """Context manager for Warabi target access."""

    def __init__(self, protocol='na+sm', provider_id=42, backend='memory'):
        self.protocol = protocol
        self.provider_id = provider_id
        self.backend = backend
        self.engine = None
        self.provider = None
        self.client = None
        self.target = None

    def __enter__(self):
        """Set up Warabi resources."""
        self.engine = mochi.margo.Engine(self.protocol, mochi.margo.server)

        config = {"target": {"type": self.backend}}
        if self.backend == "abtio":
            config["target"]["config"] = {
                "path": "/tmp/warabi_context.dat",
                "create_if_missing": True
            }

        self.provider = Provider(
            engine=self.engine,
            provider_id=self.provider_id,
            config=config
        )

        self.client = Client(engine=self.engine)
        self.target = self.client.make_target_handle(
            str(self.engine.addr()),
            self.provider_id
        )

        return self.target

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Clean up Warabi resources."""
        if self.engine:
            self.engine.finalize()
        return False  # Don't suppress exceptions


# Example 1: Basic usage
print("=== Example 1: Basic Context Manager ===")
with WarabiTarget() as target:
    # Use the target
    data = b"Context manager data"
    region = target.create_and_write(data, persist=False)
    print(f"Created region: {region}")

    result = target.read(region, 0, len(data))
    print(f"Read data: {result}")
    assert result == data

# Resources automatically cleaned up
print("Resources cleaned up automatically\n")


# Example 2: With different backend
print("=== Example 2: Different Backend ===")
with WarabiTarget(backend='abtio') as target:
    data = b"Persistent data"
    region = target.create_and_write(data, persist=True)
    print(f"Created persistent region: {region}")

print("ABT-IO backend cleaned up\n")


# Example 3: Error handling with context manager
print("=== Example 3: Error Handling ===")
try:
    with WarabiTarget() as target:
        data = b"Error test data"
        region = target.create_and_write(data)
        print(f"Created region: {region}")

        # Simulate an error
        raise ValueError("Simulated error!")

except ValueError as e:
    print(f"Error occurred: {e}")

# Resources still cleaned up despite error
print("Resources cleaned up despite error\n")


# Example 4: Multiple regions with cleanup
print("=== Example 4: Automatic Region Cleanup ===")

class WarabiSession(WarabiTarget):
    """Enhanced context manager that tracks and cleans up regions."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.regions = []

    def __enter__(self):
        self.target = super().__enter__()
        return self

    def create_region(self, size):
        """Create a region and track it for cleanup."""
        region = self.target.create(size)
        self.regions.append(region)
        return region

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Clean up all regions before finalizing."""
        print(f"Cleaning up {len(self.regions)} regions...")
        for region in self.regions:
            try:
                self.target.erase(region)
            except Exception as e:
                print(f"Error cleaning up region {region}: {e}")
        return super().__exit__(exc_type, exc_val, exc_tb)


with WarabiSession() as session:
    # Create multiple regions
    for i in range(5):
        region = session.create_region(1024)
        data = f"Data for region {i}".encode()
        session.target.write(region, 0, data, persist=False)
        print(f"Created and wrote to region {i}: {region}")

# All regions automatically erased
print("All regions cleaned up automatically\n")

print("Context manager examples completed!")
