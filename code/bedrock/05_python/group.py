#!/usr/bin/env python
"""
Example of working with Bedrock service groups using Flock.
"""
from mochi.bedrock import Client
import sys

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <flock_group_file>")
    sys.exit(1)

group_file = sys.argv[1]

# Create a Bedrock client
client = Client("na+sm")

# Create a handle to the service group
# This assumes all members have Bedrock provider at ID 0
group = client.make_service_group_handle(group_file, provider_id=0)

print(f"Connected to service group")
print(f"Group size: {group.size}")

# Refresh group membership (in case it changed)
group.refresh()

# Access individual members
print("\n=== Service Members ===")
for i in range(group.size):
    service = group[i]
    config = service.config
    num_providers = len(config.get('providers', []))
    print(f"Member {i}:")
    print(f"  Address: {service.address}")
    print(f"  Providers: {num_providers}")

# Query all members for their provider names
print("\n=== Provider Names from All Members ===")
jx9_script = """
$result = [];
foreach($__config__['providers'] as $provider) {
    $result[] = $provider['name'];
}
return $result;
"""

for i in range(group.size):
    service = group[i]
    provider_names = service.query(jx9_script)
    print(f"Member {i}: {provider_names}")

# Clean up
client.finalize()
