import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

# Create a Margo engine
engine = mochi.margo.Engine("na+sm", mochi.margo.server)

# Start a Warabi provider with memory backend
provider = Provider(
    engine=engine,
    provider_id=42,
    config={
        "target": {
            "type": "memory"
        }
    }
)

# Create a client
client = Client(engine=engine)

# Get a target handle
target = client.make_target_handle(
    address=str(engine.addr()),
    provider_id=42
)

# Create a region and write data
data = b"Hello, Warabi!"
region = target.create_and_write(data, persist=True)
print(f"Created region: {region}")

# Read the data back
result = target.read(region, offset=0, size=len(data))
print(f"Read data: {result}")

# Verify
assert result == data
print("Success!")

# Cleanup
engine.finalize()
