import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Asynchronous create
print("Creating region asynchronously...")
create_req = target.create_async(size=4096)
# Do other work while region is being created
print("Doing other work...")
# Wait for creation to complete
region = create_req.wait()
print(f"Region created: {region}")

# Asynchronous write
data = b"Async write data"
print("Writing asynchronously...")
write_req = target.write_async(region, offset=0, data=data, persist=False)
# Do other work
print("Doing other work while writing...")
# Wait for write to complete
write_req.wait()
print("Write completed")

# Asynchronous read
buffer = bytearray(len(data))
print("Reading asynchronously...")
read_req = target.read_async(region, offset=0, buffer=buffer)
# Do other work
print("Doing other work while reading...")
# Wait for read to complete
read_req.wait()
print(f"Read completed: {bytes(buffer)}")
assert bytes(buffer) == data

# Asynchronous persist
print("Persisting asynchronously...")
persist_req = target.persist_async(region, offset=0, size=len(data))
# Do other work
print("Doing other work while persisting...")
# Wait for persist to complete
persist_req.wait()
print("Persist completed")

# Multiple concurrent async operations
print("\nConcurrent async operations:")
regions = []
requests = []

# Issue multiple async creates
for i in range(5):
    req = target.create_async(size=1024)
    requests.append(req)

# Wait for all to complete
for i, req in enumerate(requests):
    region = req.wait()
    regions.append(region)
    print(f"Region {i} created: {region}")

# Write to all regions concurrently
write_requests = []
for i, region in enumerate(regions):
    data = f"Data for region {i}".encode()
    req = target.write_async(region, offset=0, data=data, persist=False)
    write_requests.append(req)

# Wait for all writes
for req in write_requests:
    req.wait()
print("All writes completed")

# Test completion without waiting
print("\nTesting completion status:")
req = target.create_async(size=512)
while not req.test():
    print("Not yet complete, doing other work...")
    # In real code, do actual work here
    pass
region = req.wait()  # Get the result
print(f"Async create tested and completed: {region}")

engine.finalize()
