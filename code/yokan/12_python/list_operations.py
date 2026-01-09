from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client
import mochi.yokan.mode as mode

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Populate database with test data
for i in range(20):
    key = f"item:{i:03d}"
    value = f"value_{i}"
    db.put(key=key, value=value)

# List up to 5 keys with a given prefix
keys = [bytearray(16) for i  in range(5)]
lengths = db.list_keys(
    keys,
    from_key="item:006",  # Starting point
    filter="item:00",     # Prefix
    mode=mode.YOKAN_MODE_DEFAULT
)
print(f"Keys starting with 'item:00'")
for k, l in zip(keys, lengths):
    print(f"  - {k[:l].decode()}")

# List keys and values together
keyvals = [[bytearray(16), bytearray(16)] for i in range(5)]
lengths = db.list_keyvals(
    keyvals,
    from_key="item:003",
    filter="",
    mode=mode.YOKAN_MODE_INCLUSIVE  # Include the starting key
)
print(f"\nKey/value pairs from 'item:003':")
for (key, value), (ksize, vsize) in zip(keyvals, lengths):
    print(f"  - {key[:ksize].decode()} = {value[:vsize].decode()}")

# List keys with a filter (remove prefix from results)
keys = [bytearray(16) for i  in range(5)]
lengths = db.list_keys(
    keys,
    from_key="item:006",
    filter="item:00",
    mode=mode.YOKAN_MODE_NO_PREFIX
)
print(f"Keys starting with 'item:00', with prefix removed")
for k, l in zip(keys, lengths):
    print(f"  - {k[:l].decode()}")

# List keys and values together, using a single buffer for each
keys = bytearray(128)
values = bytearray(128)
lengths = db.list_keyvals_packed(
    keys, values,
    count=5,
    from_key="item:003",
    filter="",
    mode=mode.YOKAN_MODE_INCLUSIVE  # Include the starting key
)
print(f"Packed key/value retrieved:")
key_offset = 0
val_offset = 0
for kl, vl in lengths:
    print(f"  - {keys[key_offset:key_offset+kl]} = {values[val_offset:val_offset+vl]}")
    key_offset += kl
    val_offset += vl

engine.finalize()
