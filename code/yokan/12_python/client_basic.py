from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Put operation
db.put(key="user:1", value="Alice")
print("Stored user:1 = Alice")

# Get operation
value = db.get(key="user:1")
print(f"Retrieved: {value}")

# Exists operation
exists = db.exists(key="user:1")
print(f"Key exists: {exists}")

# Length operation
length = db.length(key="user:1")
print(f"Value length: {length} bytes")

# Erase operation
db.erase(key="user:1")
print("Key erased")

# Verify erasure
exists_after = db.exists(key="user:1")
print(f"Key exists after erase: {exists_after}")

engine.finalize()
