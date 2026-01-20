#!/usr/bin/env python
"""
Example of using Flock with Bedrock Python bindings.
"""
from mochi.bedrock.server import Server
from mochi.bedrock.client import Client
import time

# Configuration with Flock
config = {
    "libraries": [
        "libflock-bedrock-module.so",
        "libyokan-bedrock-module.so"
        ],
    "providers": [
        {
            "name": "my_group",
            "type": "flock",
            "provider_id": 1,
            "config": {
                "bootstrap": "self",
                "group": {
                    "type": "static",
                    "config": {}
                },
                "file": "/tmp/my_group.flock"
            }
        },
        {
            "name": "my_database",
            "type": "yokan",
            "provider_id": 42,
            "config": {
                "database": {"type": "map"}
            }
        }
    ]
}

# Start server with Flock
print("Starting Bedrock server with Flock...")
server = Server("na+sm", config=config)
address = server.margo.engine.address
print(f"Server started at {address}")

# Connect as client
print("\nConnecting to service...")
client = Client("na+sm")
service = client.make_service_handle(address, provider_id=0)

# Query configuration
config_result = service.config
print(f"\nService has {len(config_result['providers'])} providers:")
for provider in config_result['providers']:
    print(f"  - {provider['name']} (type={provider['type']}, id={provider['provider_id']})")

# Use ServiceGroupHandle to interact with Flock group
# (assuming you have the group file path)
group_file = "/tmp/my_group.flock"
try:
    group = client.make_service_group_handle_from_flock(group_file, provider_id=0)
    print(f"\nFlock group size: {group.size}")
except Exception as e:
    print(f"\nNote: Could not create group handle (single member group): {e}")
