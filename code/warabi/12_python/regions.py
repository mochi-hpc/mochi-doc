import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Create a 1MB region
region = target.create(size=1024 * 1024)
print(f"Created 1MB region: {region}")

# Write data at different offsets
chunk1 = b"First chunk of data"
chunk2 = b"Second chunk"
chunk3 = b"Third chunk at the end"

target.write(region, offset=0, data=chunk1, persist=False)
target.write(region, offset=1000, data=chunk2, persist=False)
target.write(region, offset=1000000, data=chunk3, persist=False)

print("Wrote data at offsets: 0, 1000, 1000000")

# Read back specific chunks
result1 = target.read(region, offset=0, size=len(chunk1))
result2 = target.read(region, offset=1000, size=len(chunk2))
result3 = target.read(region, offset=1000000, size=len(chunk3))

assert result1 == chunk1
assert result2 == chunk2
assert result3 == chunk3
print("All chunks verified!")

# Persist the entire region
target.persist(region, offset=0, size=1024 * 1024)
print("Region persisted")

# Create multiple regions for different purposes
metadata_region = target.create(size=4096)  # 4KB for metadata
data_region = target.create(size=10 * 1024 * 1024)  # 10MB for data

print(f"Metadata region: {metadata_region}")
print(f"Data region: {data_region}")

# Clean up
target.erase(region)
target.erase(metadata_region)
target.erase(data_region)

engine.finalize()
