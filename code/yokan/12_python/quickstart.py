from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client

# Create a Margo engine
engine = Engine('tcp')

# Start a Yokan provider
provider = Provider(
    engine=engine,
    provider_id=42,
    config='{"database":{"type":"map"}}'
)

# Create a client
client = Client(engine=engine)

# Get a database handle
db = client.make_database_handle(
    address=engine.addr(),
    provider_id=42
)

# Put a key/value pair
db.put(key="greeting", value="Hello, Yokan!")

# Get the value back
size = db.length(key="greeting")
value = bytearray(size)
db.get(key="greeting", value=value)
print(f"Retrieved: {value.decode()}")

# Count total entries
count = db.count()
print(f"Total entries: {count}")

# Cleanup
engine.finalize()
