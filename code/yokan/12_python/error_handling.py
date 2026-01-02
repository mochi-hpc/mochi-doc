from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client, Exception as ClientException
from mochi.yokan.mode import Mode

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Example 1: Handling missing keys
try:
    value = db.get(key="nonexistent_key")
except ClientException as e:
    print(f"Expected error - key not found: {e}")

# Example 2: Checking before getting
if db.exists(key="safe_key"):
    value = db.get(key="safe_key")
else:
    print("Key doesn't exist, using default value")
    value = "default"

# Example 3: Handling unsupported modes
# (This would fail if the backend doesn't support WAIT mode)
try:
    db.put(key="test", value="data")
    # Some backends may not support all modes
    # value = db.get(key="test", mode=Mode.SOME_UNSUPPORTED_MODE)
    print("Mode check: using supported modes only")
except ClientException as e:
    print(f"Mode not supported: {e}")

# Example 4: Graceful degradation
def safe_get(db, key, default=None):
    """Get a value with a default fallback."""
    try:
        return db.get(key=key)
    except ClientException:
        return default

result = safe_get(db, "missing_key", default="Not found")
print(f"Safe get result: {result}")

# Example 5: Batch operation error handling
keys = ["key1", "key2", "key3"]
db.put(key="key1", value="value1")
db.put(key="key3", value="value3")
# key2 is missing

# get_multi returns None for missing keys instead of raising
values = db.get_multi(keys=keys)
print(f"Batch get with missing key: {values}")
# Result: ['value1', None, 'value3']

engine.finalize()
