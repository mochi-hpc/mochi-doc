from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client, Exception
import mochi.yokan.mode as mode

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                    config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# APPEND mode: Append to existing value
db.put(key="log", value="Entry 1\n")
db.put(key="log", value="Entry 2\n", mode=mode.YOKAN_MODE_APPEND)
db.put(key="log", value="Entry 3\n", mode=mode.YOKAN_MODE_APPEND)
value = bytearray(24)
db.get(key="log", value=value)
print(f"Appended log:\n{value.decode()}")

# CONSUME mode: Get and erase in one operation
db.put(key="task", value="process_this")
value = bytearray(12)
db.get(key="task", value=value, mode=mode.YOKAN_MODE_CONSUME)
print(f"Consumed: {value}")
print(f"Still exists: {db.exists(key='task')}")  # False

# NEW_ONLY mode: Only put if key doesn't exist
try:
    db.put(key="counter", value="1", mode=mode.YOKAN_MODE_NEW_ONLY)
except Exception as e:
    print(f"This should not be reached")
print("First put succeeded")
try:
    db.put(key="counter", value="2", mode=mode.YOKAN_MODE_NEW_ONLY)
except Exception as e:
    print(f"Second put failed (expected): {e}")

# EXIST_ONLY mode: Only put if key already exists
try:
    db.put(key="new_key", value="value", mode=mode.YOKAN_MODE_EXIST_ONLY)
except Exception:
    print("Put failed on non-existent key (expected)")


# Combining modes
db.put(key="multi_mode", value="data")
value = bytearray(4)
db.get(key="multi_mode", value=value, mode=mode.YOKAN_MODE_CONSUME | mode.YOKAN_MODE_NO_RDMA)
print(f"Consumed with NO_RDMA: {value}")

engine.finalize()
