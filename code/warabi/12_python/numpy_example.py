import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client
import numpy as np

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Store NumPy arrays directly
print("=== Storing NumPy Arrays ===")

# Small array
small_array = np.array([1, 2, 3, 4, 5], dtype=np.float64)
region1 = target.create_and_write(small_array.tobytes(), persist=False)
print(f"Stored small array: shape={small_array.shape}, dtype={small_array.dtype}")

# Large 2D array
large_array = np.random.rand(1000, 1000).astype(np.float32)
region2 = target.create_and_write(large_array.tobytes(), persist=False)
print(f"Stored large array: shape={large_array.shape}, dtype={large_array.dtype}")

# 3D array
array_3d = np.random.randint(0, 255, (100, 100, 3), dtype=np.uint8)
region3 = target.create_and_write(array_3d.tobytes(), persist=False)
print(f"Stored 3D array: shape={array_3d.shape}, dtype={array_3d.dtype}")

# Retrieve and reconstruct arrays
print("\n=== Retrieving NumPy Arrays ===")

# Small array
data1 = target.read(region1, 0, small_array.nbytes)
reconstructed1 = np.frombuffer(data1, dtype=np.float64)
assert np.array_equal(small_array, reconstructed1)
print(f"Reconstructed small array: {reconstructed1}")

# Large array
data2 = target.read(region2, 0, large_array.nbytes)
reconstructed2 = np.frombuffer(data2, dtype=np.float32).reshape(1000, 1000)
assert np.array_equal(large_array, reconstructed2)
print(f"Reconstructed large array: shape={reconstructed2.shape}")

# 3D array
data3 = target.read(region3, 0, array_3d.nbytes)
reconstructed3 = np.frombuffer(data3, dtype=np.uint8).reshape(100, 100, 3)
assert np.array_equal(array_3d, reconstructed3)
print(f"Reconstructed 3D array: shape={reconstructed3.shape}")

# Efficient reading into preallocated arrays
print("\n=== Zero-Copy with Pre-allocated Buffers ===")

# Create array to read into
buffer_array = np.zeros_like(large_array)
target.read_into(region2, 0, buffer_array.data)
assert np.array_equal(large_array, buffer_array)
print("Read into preallocated array (zero-copy)")

# Batch storage of multiple arrays
print("\n=== Batch Storage ===")

arrays = [
    np.random.rand(100, 100) for _ in range(10)
]

regions = []
for i, arr in enumerate(arrays):
    region = target.create_and_write(arr.tobytes(), persist=False)
    regions.append((region, arr.shape, arr.dtype))
    print(f"Stored array {i}: shape={arr.shape}")

# Retrieve all arrays
print("\n=== Batch Retrieval ===")
retrieved_arrays = []
for i, (region, shape, dtype) in enumerate(regions):
    size = np.prod(shape) * np.dtype(dtype).itemsize
    data = target.read(region, 0, size)
    arr = np.frombuffer(data, dtype=dtype).reshape(shape)
    retrieved_arrays.append(arr)
    assert np.array_equal(arrays[i], arr)
    print(f"Retrieved array {i}: verified")

print("\nAll NumPy arrays successfully stored and retrieved!")

engine.finalize()
