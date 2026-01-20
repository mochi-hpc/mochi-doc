from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client, Exception as ClientException
import mochi.yokan.mode as mode

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                    config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

value = bytearray(16)

# Example 1: Handling missing keys
try:
    db.get(key="nonexistent_key", value=value)
except ClientException as e:
    print(f"Expected error - key not found: {e}")

# Example 2: Checking before getting
if db.exists(key="safe_key"):
    db.get(key="safe_key", value=value)
else:
    print("Key doesn't exist")

# Example 3: Handling unsupported modes
# (This would fail if the backend doesn't support WAIT mode)
try:
    db.put(key="test", value="data")
    # Some backends may not support all modes
    # db.get(key="test", value=value, mode=mode.SOME_UNSUPPORTED_MODE)
    print("Mode check: using supported modes only")
except ClientException as e:
    print(f"Mode not supported: {e}")

engine.finalize()
