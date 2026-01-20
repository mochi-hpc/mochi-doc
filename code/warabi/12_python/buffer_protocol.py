import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Using bytes (simple but creates copies)
data_bytes = b"Hello from bytes"
region1 = target.create_and_write(data_bytes)
print("Wrote using bytes")

# Using bytearray (more efficient for large data)
data_bytearray = bytearray(b"Hello from bytearray")
region2 = target.create_and_write(data_bytearray)
print("Wrote using bytearray")

# Reading into pre-allocated buffer (zero-copy)
buffer = bytearray(len(data_bytes))
target.read_into(region1, offset=0, buffer=buffer)
print(f"Read into buffer: {bytes(buffer)}")
assert bytes(buffer) == data_bytes

# Using memoryview for large data
large_data = bytearray(10 * 1024 * 1024)  # 10MB
large_data[0:5] = b"START"
large_data[-5:] = b"END!!"

region3 = target.create_and_write(memoryview(large_data))
print(f"Wrote 10MB using memoryview (zero-copy)")

# Read back into preallocated buffer
read_buffer = bytearray(10 * 1024 * 1024)
target.read_into(region3, offset=0, buffer=read_buffer)

# Verify
assert read_buffer[0:5] == b"START"
assert read_buffer[-5:] == b"END!!"
print("Large data verified!")

# Partial reads with buffer
partial_buffer = bytearray(5)
target.read_into(region3, offset=0, buffer=partial_buffer)
assert bytes(partial_buffer) == b"START"
print("Partial read successful")

engine.finalize()
