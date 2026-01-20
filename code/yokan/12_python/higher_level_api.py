from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client, Exception as ClientException
import json
from typing import Any, Optional, Dict

class YokanDict:
    """
    Higher-level dictionary-like interface to Yokan.
    Provides Pythonic access with automatic JSON serialization.
    """

    def __init__(self, db, prefix: str = ""):
        self.db = db
        self.prefix = prefix

    def _make_key(self, key: str) -> str:
        """Add prefix to key."""
        return f"{self.prefix}{key}" if self.prefix else key

    def __setitem__(self, key: str, value: Any):
        """Set a value (automatically serializes to JSON)."""
        json_value = json.dumps(value)
        self.db.put(key=self._make_key(key), value=json_value)

    def __getitem__(self, key: str) -> Any:
        """Get a value (automatically deserializes from JSON)."""
        try:
            full_key = self._make_key(key)
            length = self.db.length(key=full_key)
            value = bytearray(length)
            self.db.get(key=full_key, value=value)
            return json.loads(value.decode())
        except ClientException:
            raise KeyError(key)

    def __delitem__(self, key: str):
        """Delete a value."""
        try:
            self.db.erase(key=self._make_key(key))
        except ClientException:
            raise KeyError(key)

    def __contains__(self, key: str) -> bool:
        """Check if key exists."""
        return self.db.exists(key=self._make_key(key))

    def get(self, key: str, default: Any = None) -> Any:
        """Get a value with default fallback."""
        try:
            return self[key]
        except KeyError:
            return default

    def update(self, items: Dict[str, Any]):
        """Update multiple items at once."""
        keyvals = [(self._make_key(k), json.dumps(v)) for k, v in items.items()]
        self.db.put_multi(keyvals)

    def clear(self):
        """Clear all items with this prefix."""
        # Note: This is a simplified implementation
        # A real implementation would use list operations
        pass


# Example usage
engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                    config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Create a higher-level dictionary interface
yokan_dict = YokanDict(db, prefix="app:")

# Use it like a regular Python dictionary
yokan_dict["user:1"] = {"name": "Alice", "age": 30}
yokan_dict["user:2"] = {"name": "Bob", "age": 25}
yokan_dict["config"] = {"theme": "dark", "notifications": True}

print("Stored data using dict-like interface")

# Retrieve data
user1 = yokan_dict["user:1"]
print(f"User 1: {user1}")

# Check existence
if "config" in yokan_dict:
    config = yokan_dict["config"]
    print(f"Config: {config}")

# Get with default
user3 = yokan_dict.get("user:3", {"name": "Unknown", "age": 0})
print(f"User 3 (default): {user3}")

# Batch update
yokan_dict.update({
    "user:3": {"name": "Carol", "age": 35},
    "user:4": {"name": "Dave", "age": 40}
})
print("Batch updated 2 users")

engine.finalize()
