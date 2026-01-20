import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

engine = mochi.margo.Engine("na+sm", mochi.margo.server)

# Use ABT-IO backend for persistent storage
provider = Provider(
    engine=engine,
    provider_id=42,
    config={
        "target": {
            "type": "abtio",
            "config": {
                "path": "/tmp/warabi_persist_demo.dat",
                "create_if_missing": True
            }
        }
    }
)

client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Strategy 1: Immediate persistence
print("=== Immediate Persistence ===")
data1 = b"Immediately persisted data"
region1 = target.create_and_write(data1, persist=True)
print("Data written and persisted immediately")

# Strategy 2: Explicit persistence
print("\n=== Explicit Persistence ===")
region2 = target.create(size=4096)
data2 = b"Write first, persist later"
target.write(region2, offset=0, data=data2, persist=False)
print("Data written (not yet persisted)")
# Do more writes if needed
target.persist(region2, offset=0, size=len(data2))
print("Data now persisted explicitly")

# Strategy 3: Batch persistence
print("\n=== Batch Persistence ===")
region3 = target.create(size=10240)

# Multiple writes without persistence
chunks = [
    (0, b"Chunk 1"),
    (100, b"Chunk 2"),
    (200, b"Chunk 3"),
    (300, b"Chunk 4"),
]

for offset, data in chunks:
    target.write(region3, offset=offset, data=data, persist=False)
    print(f"Wrote chunk at offset {offset} (not persisted)")

# Persist all at once
target.persist(region3, offset=0, size=400)
print("All chunks persisted in one operation")

# Strategy 4: Asynchronous persistence
print("\n=== Asynchronous Persistence ===")
region4 = target.create(size=4096)
data4 = b"Async persisted data"

# Write without persistence
target.write(region4, offset=0, data=data4, persist=False)
print("Data written (not persisted)")

# Async persist (non-blocking)
req = target.persist_async(region4, offset=0, size=len(data4))
print("Async persist issued, doing other work...")
# Do other work here
req.wait()
print("Async persist completed")

# Verification: Read back all data
print("\n=== Verification ===")
result1 = target.read(region1, 0, len(data1))
assert result1 == data1
print("Region 1 verified")

result2 = target.read(region2, 0, len(data2))
assert result2 == data2
print("Region 2 verified")

for offset, expected in chunks:
    result = target.read(region3, offset, len(expected))
    assert result == expected
print("Region 3 (all chunks) verified")

result4 = target.read(region4, 0, len(data4))
assert result4 == data4
print("Region 4 verified")

print("\nAll data successfully persisted and verified!")

engine.finalize()
