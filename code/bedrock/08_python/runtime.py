#!/usr/bin/env python
"""
Example of runtime configuration manipulation with Bedrock Python API.
"""
from mochi.bedrock.client import Client
import time
import json
import sys

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <server_address>")
    sys.exit(1)

address = sys.argv[1]

# Connect as a client
print("\nConnecting to server...")
client = Client("na+sm")
service = client.make_service_handle(address, provider_id=0)

# Show initial configuration
print("\n=== Initial Configuration ===")
config = service.config
print(f"Providers: {len(config.get('providers', []))}")

# Load a module
print("\n=== Loading Module ===")
service.load_module("libyokan-bedrock-module.so")

# Add a new pool
print("\n=== Adding Pool ===")
pool_config = {
    "name": "dynamic_pool",
    "kind": "fifo_wait",
    "access": "mpmc"
}
service.add_pool(pool_config)
print("Added pool: dynamic_pool")

# Add a new execution stream
print("\n=== Adding Execution Stream ===")
xstream_config = {
    "name": "dynamic_xstream",
    "scheduler": {
        "type": "basic_wait",
        "pools": ["dynamic_pool"]
    }
}
service.add_xstream(xstream_config)
print("Added xstream: dynamic_xstream")

# Add a new provider
print("\n=== Adding Provider ===")
provider_config = {
    "name": "runtime_database",
    "type": "yokan",
    "provider_id": 100,
    "dependencies": {
        "pool": "dynamic_pool",
    },
    "config": {
        "database": {
            "type": "map"
        }
    }
}
new_provider_id = service.add_provider(provider_config)
print(f"Added provider with ID: {new_provider_id}")

# Query updated configuration
print("\n=== Updated Configuration ===")
config = service.config
print(f"Providers: {len(config.get('providers', []))}")
print("\nProvider list:")
for provider in config.get('providers', []):
    print(f"  - {provider['name']} (type={provider['type']}, id={provider['provider_id']})")
