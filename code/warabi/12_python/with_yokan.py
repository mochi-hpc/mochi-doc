"""
Example combining Warabi (blob storage) with Yokan (metadata storage).

This pattern is useful for applications that need both metadata management
and large blob storage.
"""

import mochi.margo
from mochi.warabi.server import Provider as WarabiProvider
from mochi.warabi.client import Client as WarabiClient
from mochi.yokan.server import Provider as YokanProvider
from mochi.yokan.client import Client as YokanClient
import json

# Setup both services
engine = mochi.margo.Engine("na+sm", mochi.margo.server)

# Warabi for blob storage
warabi_provider = WarabiProvider(
    engine=engine,
    provider_id=1,
    config={"target": {"type": "memory"}}
)

# Yokan for metadata
yokan_provider = YokanProvider(
    engine=engine,
    provider_id=2,
    config={"database": {"type": "map"}}
)

# Clients
warabi_client = WarabiClient(engine=engine)
yokan_client = YokanClient(engine=engine)

warabi_target = warabi_client.make_target_handle(str(engine.addr()), 1)
yokan_db = yokan_client.make_database_handle(str(engine.addr()), 2)

print("=== Combined Warabi + Yokan Usage ===\n")

# Store large blob in Warabi
print("Storing large blob in Warabi...")
large_data = b"X" * 10000  # 10KB of data
blob_region = warabi_target.create_and_write(large_data, persist=True)
print(f"Blob stored in region: {blob_region}")

# Store metadata in Yokan
print("\nStoring metadata in Yokan...")
metadata = {
    "name": "large_dataset.bin",
    "size": len(large_data),
    "region_id": str(blob_region),
    "content_type": "application/octet-stream",
    "created": "2024-01-01T00:00:00Z"
}

metadata_key = "file:large_dataset"
yokan_db.put(
    key=metadata_key,
    value=json.dumps(metadata)
)
print(f"Metadata stored with key: {metadata_key}")

# Retrieve: First get metadata from Yokan
print("\n=== Retrieval ===")
print("Getting metadata from Yokan...")
metadata_json = yokan_db.get(key=metadata_key)
retrieved_metadata = json.loads(metadata_json)
print(f"Metadata: {retrieved_metadata}")

# Then retrieve blob from Warabi using region ID
print("\nGetting blob from Warabi...")
blob_data = warabi_target.read(
    int(retrieved_metadata["region_id"]),
    0,
    retrieved_metadata["size"]
)
print(f"Retrieved blob size: {len(blob_data)} bytes")
assert blob_data == large_data

# List all files (metadata in Yokan)
print("\n=== Listing Files ===")
keys = yokan_db.list_keys(from_key="file:", count=10)
print(f"Files in storage: {keys}")

# Delete pattern: Remove metadata and blob
print("\n=== Deletion ===")
print("Deleting file...")

# Get metadata to find region ID
metadata_json = yokan_db.get(key=metadata_key)
metadata = json.loads(metadata_json)

# Delete blob from Warabi
warabi_target.erase(int(metadata["region_id"]))
print("Blob deleted from Warabi")

# Delete metadata from Yokan
yokan_db.erase(key=metadata_key)
print("Metadata deleted from Yokan")

print("\nCombined usage example completed!")
print("Pattern: Yokan for metadata + Warabi for blobs")

engine.finalize()
