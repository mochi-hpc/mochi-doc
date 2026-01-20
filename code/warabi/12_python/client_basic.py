import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Create a region
region = target.create(size=4096)
print(f"Created region: {region}")

# Write data to the region
data = b"Hello, Warabi from Python!"
target.write(region, offset=0, data=data, persist=False)
print(f"Wrote {len(data)} bytes")

# Read data back
result = target.read(region, offset=0, size=len(data))
print(f"Read back: {result}")
assert result == data

# Create and write in one operation
data2 = b"Combined operation!"
region2 = target.create_and_write(data2, persist=True)
print(f"Created and wrote region: {region2}")

# Verify
result2 = target.read(region2, offset=0, size=len(data2))
assert result2 == data2

# Persist data explicitly
target.persist(region, offset=0, size=len(data))
print("Data persisted")

# Erase a region
target.erase(region)
print("Region erased")

engine.finalize()
