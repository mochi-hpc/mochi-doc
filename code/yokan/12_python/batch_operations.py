from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Put multiple key/value pairs at once
keys = ["user:1", "user:2", "user:3"]
values = ["Alice", "Bob", "Carol"]
db.put_multi(list(zip(keys, values)))
print(f"Stored {len(keys)} key/value pairs")

# Check existence of multiple keys
new_keys = keys + ["user:4", "user:5"]
existence = db.exists_multi(keys=new_keys)
print(f"Existence check: {existence}")  # [True, True, True, False, False]

# Get lengths of multiple values
lengths = db.length_multi(keys=keys)
print(f"Value lengths: {lengths}")

# Get multiple values at once
retrieved_values = [bytearray(l) for l in lengths]
db.get_multi(list(zip(keys, retrieved_values)))
print(f"Retrieved: {retrieved_values}")

# Erase multiple keys
db.erase_multi(keys=keys)
print(f"Erased {len(keys)} keys")

# Verify
count = db.count()
print(f"Remaining entries: {count}")

engine.finalize()
