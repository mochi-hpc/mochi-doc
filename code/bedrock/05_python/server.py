#!/usr/bin/env python
"""
Example of starting a Bedrock service from Python.
"""
from mochi.bedrock import Server
import time

# Configuration as a Python dictionary
config = {
    "margo": {
        "argobots": {
            "pools": [
                {"name": "my_pool", "kind": "fifo_wait", "access": "mpmc"}
            ]
        }
    },
    "libraries": {
        "yokan": "libyokan-bedrock-module.so"
    },
    "providers": [
        {
            "name": "my_database",
            "type": "yokan",
            "provider_id": 42,
            "pool": "__primary__",
            "config": {
                "database": {
                    "type": "map"
                }
            }
        }
    ]
}

# Start the Bedrock server
server = Server("na+sm", config=config)

print(f"Bedrock server started at {server.margo.address}")
print("Press Ctrl+C to stop...")

try:
    # Keep the server running
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("\nShutting down...")

# Clean up
server.finalize()
print("Server finalized")
