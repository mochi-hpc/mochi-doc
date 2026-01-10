import mochi.margo
from mochi.warabi.server import Provider

engine = mochi.margo.Engine("na+sm", mochi.margo.server)

# Memory backend (volatile, fast)
memory_provider = Provider(
    engine=engine,
    provider_id=1,
    config={
        "target": {
            "type": "memory"
        }
    }
)
print("Memory backend provider created (ID: 1)")

# ABT-IO file-based backend (persistent)
abtio_provider = Provider(
    engine=engine,
    provider_id=2,
    config={
        "target": {
            "type": "abtio",
            "config": {
                "path": "/tmp/warabi_storage.dat",
                "create_if_missing": True
            }
        }
    }
)
print("ABT-IO backend provider created (ID: 2)")

# Persistent memory backend (if available)
try:
    pmem_provider = Provider(
        engine=engine,
        provider_id=3,
        config={
            "target": {
                "type": "pmem",
                "config": {
                    "path": "/mnt/pmem/warabi_data",
                    "create_if_missing_with_size": 1073741824  # 1GB
                }
            }
        }
    )
    print("PMem backend provider created (ID: 3)")
except Exception as e:
    print(f"PMem backend not available: {e}")

engine.finalize()
