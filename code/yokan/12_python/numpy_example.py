from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client
import numpy as np

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Store NumPy arrays directly
array1 = np.array([1, 2, 3, 4, 5], dtype=np.float64)
array2 = np.random.rand(100, 100)  # Large random array

# NumPy arrays implement the buffer protocol, so they can be stored directly
db.put(key="small_array", value=array1.tobytes())
db.put(key="large_array", value=array2.tobytes())

print(f"Stored arrays:")
print(f"  small_array: shape={array1.shape}, dtype={array1.dtype}")
print(f"  large_array: shape={array2.shape}, dtype={array2.dtype}")

# Retrieve and reconstruct arrays
small_bytes = db.get(key="small_array")
small_reconstructed = np.frombuffer(small_bytes, dtype=np.float64)
print(f"\nRetrieved small array: {small_reconstructed}")

large_bytes = db.get(key="large_array")
large_reconstructed = np.frombuffer(large_bytes, dtype=np.float64).reshape(100, 100)
print(f"Retrieved large array: shape={large_reconstructed.shape}")

# Verify data integrity
assert np.array_equal(array1, small_reconstructed)
assert np.array_equal(array2, large_reconstructed)
print("\nData integrity verified!")

# Store multiple arrays with batch operations
keys = [f"array_{i}" for i in range(5)]
arrays = [np.random.rand(10) for _ in range(5)]
values = [arr.tobytes() for arr in arrays]

db.put_multi(keys=keys, values=values)
print(f"\nStored {len(keys)} arrays using batch operation")

# Retrieve multiple arrays
retrieved_bytes = db.get_multi(keys=keys)
retrieved_arrays = [
    np.frombuffer(b, dtype=np.float64) if b is not None else None
    for b in retrieved_bytes
]

print(f"Retrieved {len([a for a in retrieved_arrays if a is not None])} arrays")

engine.finalize()
