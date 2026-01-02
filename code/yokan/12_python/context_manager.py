from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client

class YokanDatabase:
    """Context manager for Yokan database access."""

    def __init__(self, protocol='tcp', provider_id=42, config=None):
        self.protocol = protocol
        self.provider_id = provider_id
        self.config = config or '{"database":{"type":"map"}}'
        self.engine = None
        self.provider = None
        self.client = None
        self.db = None

    def __enter__(self):
        """Set up Yokan resources."""
        self.engine = Engine(self.protocol)
        self.provider = Provider(
            engine=self.engine,
            provider_id=self.provider_id,
            config=self.config
        )
        self.client = Client(engine=self.engine)
        self.db = self.client.make_database_handle(
            address=self.engine.addr(),
            provider_id=self.provider_id
        )
        return self.db

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Clean up Yokan resources."""
        # Resources are automatically cleaned up
        if self.engine:
            self.engine.finalize()
        return False  # Don't suppress exceptions


# Use the context manager
with YokanDatabase() as db:
    db.put(key="message", value="Hello from context manager!")
    value = db.get(key="message")
    print(f"Retrieved: {value}")

    count = db.count()
    print(f"Total entries: {count}")

# Resources are automatically cleaned up when exiting the 'with' block
print("Context manager automatically cleaned up resources")
