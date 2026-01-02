from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client
from mochi.yokan.mode import Mode

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Populate database with test data
for i in range(10):
    key = f"item:{i:03d}"
    value = f"value_{i}"
    db.put(key=key, value=value)

# List all keys with a prefix
keys = db.list_keys(
    from_key="item:",  # Starting point (prefix)
    count=10,          # Maximum number of keys to retrieve
    mode=Mode.DEFAULT
)
print(f"Keys starting with 'item:': {keys}")

# List keys and values together
keyvals = db.list_keyvals(
    from_key="item:005",
    count=5,
    mode=Mode.INCLUSIVE  # Include the starting key
)
print(f"\nKey/value pairs from 'item:005':")
for key, value in keyvals:
    print(f"  {key} = {value}")

# List keys with a filter (remove prefix from results)
keys_no_prefix = db.list_keys(
    from_key="item:",
    count=5,
    mode=Mode.NO_PREFIX
)
print(f"\nKeys with prefix removed: {keys_no_prefix}")

engine.finalize()
