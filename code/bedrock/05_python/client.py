#!/usr/bin/env python
"""
Example of connecting to a Bedrock service from Python.
"""
from mochi.bedrock import Client
import sys

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <server_address>")
    sys.exit(1)

server_address = sys.argv[1]

# Create a Bedrock client
client = Client("na+sm")

# Create a handle to the service
service = client.make_service_handle(server_address, provider_id=0)

print(f"Connected to Bedrock service at {service.address}")
print(f"Provider ID: {service.provider_id}")

# Get the configuration
config = service.config
print("\nService configuration:")
print(f"  Number of providers: {len(config.get('providers', []))}")

# List all providers
if 'providers' in config:
    print("\nProviders:")
    for provider in config['providers']:
        print(f"  - {provider['name']} (type={provider['type']}, id={provider['provider_id']})")

# Clean up
client.finalize()
