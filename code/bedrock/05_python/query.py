#!/usr/bin/env python
"""
Example of querying a Bedrock service configuration.
"""
from mochi.bedrock import Client
import sys
import json

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <server_address>")
    sys.exit(1)

server_address = sys.argv[1]

# Create client and service handle
client = Client("na+sm")
service = client.make_service_handle(server_address, provider_id=0)

# Get complete configuration
config = service.config
print("=== Complete Configuration ===")
print(json.dumps(config, indent=2))

# Use a Jx9 query to get only provider names
jx9_script = """
$result = [];
foreach($__config__['providers'] as $provider) {
    $result[] = $provider['name'];
}
return $result;
"""

provider_names = service.query(jx9_script)
print("\n=== Provider Names (via Jx9) ===")
print(provider_names)

# Query for pools
jx9_pools = """
$result = [];
if(array_key_exists('margo', $__config__) &&
   array_key_exists('argobots', $__config__['margo']) &&
   array_key_exists('pools', $__config__['margo']['argobots'])) {
    foreach($__config__['margo']['argobots']['pools'] as $pool) {
        $result[] = $pool['name'];
    }
}
return $result;
"""

pools = service.query(jx9_pools)
print("\n=== Argobots Pools ===")
print(pools)

# Clean up
client.finalize()
