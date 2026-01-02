import mochi.margo
from mochi.warabi.server import Provider
from mochi.warabi.client import Client, Exception as WarabiException

engine = mochi.margo.Engine("na+sm", mochi.margo.server)
provider = Provider(engine=engine, provider_id=42,
                   config={"target": {"type": "memory"}})
client = Client(engine=engine)
target = client.make_target_handle(str(engine.addr()), 42)

# Example 1: Handling invalid region access
print("=== Example 1: Invalid Region ===")
try:
    # Try to read from a non-existent region
    fake_region = 99999
    data = target.read(fake_region, 0, 100)
except WarabiException as e:
    print(f"Expected error - invalid region: {e}")

# Example 2: Out of bounds access
print("\n=== Example 2: Out of Bounds ===")
region = target.create(size=1024)
try:
    # Try to read beyond region size
    data = target.read(region, offset=2000, size=100)
except WarabiException as e:
    print(f"Expected error - out of bounds: {e}")

# Example 3: Safe operations with error handling
print("\n=== Example 3: Safe Operations ===")

def safe_read(target, region, offset, size):
    """Safely read data with error handling."""
    try:
        return target.read(region, offset, size)
    except WarabiException as e:
        print(f"Read failed: {e}")
        return None

# Create and write data
region2 = target.create_and_write(b"Test data", persist=False)

# Safe read
result = safe_read(target, region2, 0, 9)
if result:
    print(f"Successfully read: {result}")

# Safe read of invalid region
result = safe_read(target, 88888, 0, 100)
if result is None:
    print("Handled invalid read gracefully")

# Example 4: Provider connection errors
print("\n=== Example 4: Connection Errors ===")
try:
    # Try to connect to non-existent provider
    bad_target = client.make_target_handle(
        "na+sm://invalid:1234",
        99
    )
    # Operations will fail
    bad_target.create(size=1024)
except WarabiException as e:
    print(f"Expected error - connection failed: {e}")

# Example 5: Backend-specific errors
print("\n=== Example 5: Backend Errors ===")
try:
    # If using file backend, this could fail with disk errors
    large_region = target.create(size=1024 * 1024 * 1024)  # 1GB
    print(f"Created large region: {large_region}")
    target.erase(large_region)
except WarabiException as e:
    print(f"Backend error: {e}")

# Example 6: Cleanup with error handling
print("\n=== Example 6: Cleanup Pattern ===")

regions_to_cleanup = []
try:
    # Create multiple regions
    for i in range(5):
        r = target.create(size=1024)
        regions_to_cleanup.append(r)
        print(f"Created region {i}: {r}")

    # Simulate an error
    raise Exception("Simulated error!")

except Exception as e:
    print(f"Error occurred: {e}")
finally:
    # Clean up regions even if error occurred
    print("Cleaning up regions...")
    for r in regions_to_cleanup:
        try:
            target.erase(r)
        except WarabiException as cleanup_error:
            print(f"Cleanup error for region {r}: {cleanup_error}")
    print("Cleanup completed")

print("\nAll error handling examples completed")

engine.finalize()
