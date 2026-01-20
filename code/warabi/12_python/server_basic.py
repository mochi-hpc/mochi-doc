import mochi.margo
from mochi.warabi.server import Provider

# Create a Margo engine
engine = mochi.margo.Engine("na+sm", mochi.margo.server)

# Create a Warabi provider with memory backend
provider = Provider(
    engine=engine,
    provider_id=42,
    config={
        "target": {
            "type": "memory"
        }
    }
)

print(f"Warabi provider started at {engine.addr()}")
print(f"Provider ID: 42")
print("Provider is now accepting requests...")

# In a real application, you would wait here
# For this example, we'll just finalize
engine.finalize()
