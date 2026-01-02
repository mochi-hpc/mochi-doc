import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client, Exception as WarabiException
from typing import Optional, Dict
import pickle

class BlobStore:
    """
    Higher-level blob storage interface with automatic serialization.
    Provides a more convenient API on top of Warabi.
    """

    def __init__(self, target):
        self.target = target
        self._metadata = {}  # In-memory metadata cache

    def put(self, key: str, data: bytes, persist: bool = True) -> None:
        """Store data with a string key."""
        region = self.target.create_and_write(data, persist=persist)
        self._metadata[key] = {
            'region': region,
            'size': len(data)
        }

    def get(self, key: str) -> Optional[bytes]:
        """Retrieve data by key."""
        if key not in self._metadata:
            return None

        meta = self._metadata[key]
        try:
            return self.target.read(meta['region'], 0, meta['size'])
        except WarabiException:
            return None

    def delete(self, key: str) -> bool:
        """Delete data by key."""
        if key not in self._metadata:
            return False

        meta = self._metadata[key]
        try:
            self.target.erase(meta['region'])
            del self._metadata[key]
            return True
        except WarabiException:
            return False

    def exists(self, key: str) -> bool:
        """Check if key exists."""
        return key in self._metadata

    def keys(self):
        """Get all keys."""
        return self._metadata.keys()


class ObjectStore(BlobStore):
    """
    Object store with automatic Python object serialization.
    """

    def put_object(self, key: str, obj, persist: bool = True) -> None:
        """Store a Python object (uses pickle)."""
        data = pickle.dumps(obj)
        self.put(key, data, persist=persist)

    def get_object(self, key: str):
        """Retrieve and deserialize a Python object."""
        data = self.get(key)
        if data is None:
            return None
        return pickle.loads(data)


# Example usage
engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

print("=== BlobStore API ===")
blob_store = BlobStore(target)

# Store blobs with string keys
blob_store.put("file1", b"Contents of file 1")
blob_store.put("file2", b"Contents of file 2")
blob_store.put("image", b"\x89PNG\r\n\x1a\n...")  # Binary data

print(f"Stored keys: {list(blob_store.keys())}")

# Retrieve blobs
data = blob_store.get("file1")
print(f"Retrieved file1: {data}")

# Check existence
if blob_store.exists("file2"):
    print("file2 exists")

# Delete
blob_store.delete("file1")
print(f"After delete: {list(blob_store.keys())}")


print("\n=== ObjectStore API ===")
obj_store = ObjectStore(target)

# Store Python objects
obj_store.put_object("config", {
    "host": "localhost",
    "port": 8080,
    "debug": True
})

obj_store.put_object("users", [
    {"name": "Alice", "id": 1},
    {"name": "Bob", "id": 2},
    {"name": "Carol", "id": 3}
])

obj_store.put_object("matrix", [[1, 2, 3], [4, 5, 6], [7, 8, 9]])

print(f"Stored objects: {list(obj_store.keys())}")

# Retrieve and use objects
config = obj_store.get_object("config")
print(f"Config: {config}")
print(f"Debug mode: {config['debug']}")

users = obj_store.get_object("users")
print(f"Users: {users}")
print(f"First user: {users[0]['name']}")

matrix = obj_store.get_object("matrix")
print(f"Matrix: {matrix}")


print("\n=== Advanced Pattern: Versioned Storage ===")

class VersionedStore(BlobStore):
    """Store with versioning support."""

    def __init__(self, target):
        super().__init__(target)
        self._versions = {}  # key -> [regions]

    def put_version(self, key: str, data: bytes) -> int:
        """Store a new version of data."""
        region = self.target.create_and_write(data, persist=True)

        if key not in self._versions:
            self._versions[key] = []

        self._versions[key].append({
            'region': region,
            'size': len(data)
        })

        return len(self._versions[key]) - 1  # Return version number

    def get_version(self, key: str, version: int = -1) -> Optional[bytes]:
        """Get a specific version (-1 for latest)."""
        if key not in self._versions:
            return None

        versions = self._versions[key]
        if not versions:
            return None

        try:
            meta = versions[version]
            return self.target.read(meta['region'], 0, meta['size'])
        except (IndexError, WarabiException):
            return None

    def version_count(self, key: str) -> int:
        """Get number of versions for a key."""
        return len(self._versions.get(key, []))


versioned = VersionedStore(target)

# Store multiple versions
v0 = versioned.put_version("document", b"Version 1 content")
v1 = versioned.put_version("document", b"Version 2 content")
v2 = versioned.put_version("document", b"Version 3 content")

print(f"Stored {versioned.version_count('document')} versions")

# Get specific versions
print(f"Version 0: {versioned.get_version('document', 0)}")
print(f"Version 1: {versioned.get_version('document', 1)}")
print(f"Latest: {versioned.get_version('document', -1)}")

print("\nHigher-level API examples completed!")

engine.finalize()
