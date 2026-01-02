import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Batch create regions
print("=== Batch Creating Regions ===")
regions = []
create_requests = []

# Issue async creates
for i in range(10):
    req = target.create_async(size=4096)
    create_requests.append(req)

# Wait for all to complete
for req in create_requests:
    region = req.wait()
    regions.append(region)

print(f"Created {len(regions)} regions")

# Batch write to all regions
print("\n=== Batch Writing ===")
write_requests = []

for i, region in enumerate(regions):
    data = f"Data for region {i}".encode()
    req = target.write_async(region, 0, data, persist=False)
    write_requests.append((req, data))

# Wait for all writes
for req, _ in write_requests:
    req.wait()

print(f"Wrote to {len(regions)} regions")

# Batch read from all regions
print("\n=== Batch Reading ===")
read_requests = []

for region in regions:
    buffer = bytearray(100)  # Large enough buffer
    req = target.read_async(region, 0, buffer)
    read_requests.append((req, buffer))

# Wait and verify
for i, (req, buffer) in enumerate(read_requests):
    req.wait()
    expected = f"Data for region {i}".encode()
    # Find actual data length
    result = bytes(buffer[:len(expected)])
    assert result == expected
    print(f"Region {i} verified: {result}")

# Batch persist
print("\n=== Batch Persisting ===")
persist_requests = []

for region in regions:
    req = target.persist_async(region, 0, 100)
    persist_requests.append(req)

for req in persist_requests:
    req.wait()

print(f"Persisted {len(regions)} regions")

# Batch erase
print("\n=== Batch Erasing ===")
for region in regions:
    target.erase(region)

print(f"Erased {len(regions)} regions")
print("Batch operations completed!")

engine.finalize()
