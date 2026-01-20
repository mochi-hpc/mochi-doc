from mochi.margo import Engine
from mochi.yokan.server import Provider
import json

# Create a Margo engine
engine = Engine('tcp')

# Configure a Yokan provider with RocksDB backend
config = {
    "database": {
        "type": "map"
    }
}

# Start the provider
provider = Provider(
    engine=engine,
    provider_id=42,
    config=json.dumps(config)
)

print(f"Yokan provider started at {engine.addr()}")
print(f"Provider ID: 42")
print("Provider is now accepting requests...")

# In a real application, you would wait here
# For this example, we'll just finalize
#engine.wait_for_finalize()
engine.finalize()
