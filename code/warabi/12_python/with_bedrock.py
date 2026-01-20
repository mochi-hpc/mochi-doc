"""
Example of using Warabi through Bedrock configuration.

This demonstrates how to configure and use Warabi when deployed
via Bedrock's configuration system.
"""
import sys

# Example Bedrock configuration for Warabi
bedrock_config = {
    "libraries": [
        "libwarabi-bedrock-module.so"
    ],
    "providers": [
        {
            "name": "warabi_memory",
            "type": "warabi",
            "provider_id": 1,
            "pool": "default_pool",
            "config": {
                "target": {
                    "type": "memory"
                }
            }
        },
        {
            "name": "warabi_persistent",
            "type": "warabi",
            "provider_id": 2,
            "pool": "io_pool",
            "config": {
                "target": {
                    "type": "abtio",
                    "config": {
                        "path": "/data/warabi_storage.dat",
                        "create_if_missing": True
                    }
                }
            }
        }
    ]
}

# Client code to use Bedrock-deployed Warabi
import mochi.margo
from mochi.warabi.client import Client

# Connect to Bedrock server
engine = mochi.margo.Engine("na+sm", mochi.margo.client)
client = Client(engine=engine)

# Connect to providers
# (In real usage, you'd get the server address from Bedrock)
server_addr = sys.argv[1]

memory_target = client.make_target_handle(server_addr, 1)
persistent_target = client.make_target_handle(server_addr, 2)

# Use memory target for temporary data
temp_data = b"Temporary data"
temp_region = memory_target.create_and_write(temp_data, persist=False)
print(f"Stored temp data in memory: {temp_region}")

# Use persistent target for important data
important_data = b"Important persistent data"
persist_region = persistent_target.create_and_write(important_data, persist=True)
print(f"Stored important data persistently: {persist_region}")

print("\nBedrock integration example (configuration shown)")
