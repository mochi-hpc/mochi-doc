import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

# Setup source and destination providers
engine = mochi.margo.Engine("na+sm", mochi.margo.server)

# Source: memory backend
source_provider = Provider(
    engine=engine,
    provider_id=1,
    config={"target": {"type": "memory"}}
)

# Destination: persistent ABT-IO backend
dest_provider = Provider(
    engine=engine,
    provider_id=2,
    config={
        "target": {
            "type": "abtio",
            "config": {
                "path": "/tmp/warabi_migrated.dat",
                "create_if_missing": True
            }
        }
    }
)

client = Client(engine=engine)
source = client.make_target_handle(str(engine.addr()), 1)
dest = client.make_target_handle(str(engine.addr()), 2)

print("=== Data Migration Pattern ===")

# Create data in source (memory)
print("\nCreating data in source (memory backend)...")
source_regions = []
for i in range(5):
    data = f"Data item {i}".encode()
    region = source.create_and_write(data, persist=False)
    source_regions.append((region, data))
    print(f"Created source region {i}: {region}")

# Migrate data to destination (persistent)
print("\nMigrating to destination (ABT-IO backend)...")
dest_regions = []

for i, (src_region, original_data) in enumerate(source_regions):
    # Read from source
    data = source.read(src_region, 0, len(original_data))

    # Write to destination with persistence
    dst_region = dest.create_and_write(data, persist=True)
    dest_regions.append(dst_region)

    print(f"Migrated region {i}: {src_region} -> {dst_region}")

# Verify migration
print("\nVerifying migration...")
for i, (dst_region, (_, original_data)) in enumerate(zip(dest_regions, source_regions)):
    data = dest.read(dst_region, 0, len(original_data))
    assert data == original_data
    print(f"Region {i} verified: {data}")

# Clean up source after successful migration
print("\nCleaning up source...")
for src_region, _ in source_regions:
    source.erase(src_region)
print("Source regions cleaned up")

print("\nMigration completed successfully!")
print(f"Data now persisted in destination backend")

engine.finalize()
