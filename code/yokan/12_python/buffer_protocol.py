from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Using strings (causes memory copies)
db.put(key="string_key", value="string_value")
value_str = db.get(key="string_key")
print(f"String approach: {value_str}")

# Using bytearray (more efficient, no copies)
key_bytes = bytearray(b"binary_key")
value_bytes = bytearray(b"\x00\x01\x02\x03\x04")  # Binary data

db.put(key=key_bytes, value=value_bytes)
retrieved_bytes = db.get(key=key_bytes)
print(f"Bytearray approach: {retrieved_bytes.hex()}")

# Using memoryview for zero-copy operations
large_data = bytearray(10000)  # Large buffer
large_data[0:5] = b"Hello"

db.put(key=b"large_key", value=memoryview(large_data))
print("Stored large data using memoryview (zero-copy)")

# Retrieve into preallocated buffer
retrieved_large = db.get(key=b"large_key")
print(f"Retrieved {len(retrieved_large)} bytes")

engine.finalize()
