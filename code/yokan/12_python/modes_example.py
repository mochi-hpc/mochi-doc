from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client
from mochi.yokan.mode import Mode

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# APPEND mode: Append to existing value
db.put(key="log", value="Entry 1\n")
db.put(key="log", value="Entry 2\n", mode=Mode.APPEND)
db.put(key="log", value="Entry 3\n", mode=Mode.APPEND)
log = db.get(key="log")
print(f"Appended log:\n{log}")

# CONSUME mode: Get and erase in one operation
db.put(key="task", value="process_this")
value = db.get(key="task", mode=Mode.CONSUME)
print(f"Consumed: {value}")
print(f"Still exists: {db.exists(key='task')}")  # False

# NEW_ONLY mode: Only put if key doesn't exist
db.put(key="counter", value="1", mode=Mode.NEW_ONLY)
print("First put succeeded")
try:
    db.put(key="counter", value="2", mode=Mode.NEW_ONLY)
except Exception as e:
    print(f"Second put failed (expected): {e}")

# EXIST_ONLY mode: Only put if key already exists
try:
    db.put(key="new_key", value="value", mode=Mode.EXIST_ONLY)
except Exception:
    print("Put failed on non-existent key (expected)")

db.put(key="existing", value="old")
db.put(key="existing", value="new", mode=Mode.EXIST_ONLY)
print(f"Updated existing key: {db.get(key='existing')}")

# Combining modes
db.put(key="multi_mode", value="data")
value = db.get(key="multi_mode", mode=Mode.CONSUME | Mode.NO_RDMA)
print(f"Consumed with NO_RDMA: {value}")

engine.finalize()
