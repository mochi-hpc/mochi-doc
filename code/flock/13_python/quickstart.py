"""
Quickstart example for Flock Python bindings.
Demonstrates basic server and client setup.
"""

import mochi.flock.server as server
import mochi.flock.client as client
from mochi.flock.view import GroupView
import pymargo.core
import json

# Create Margo engine
engine = pymargo.core.Engine("na+sm", pymargo.core.server)

# Create initial group view
initial_view = GroupView()
initial_view.members.add(str(engine.address), 42)

# Configure Flock provider with static backend
config = {
    "group": {
        "type": "static",
        "config": {}
    }
}

# Create Flock provider
provider = server.Provider(
    engine=engine,
    provider_id=42,
    config=json.dumps(config),
    initial_view=initial_view
)

print(f"Flock provider created at {engine.address} with provider_id 42")

# Create client
flock_client = client.Client(engine)

# Create group handle
group = flock_client.make_group_handle(
    address=str(engine.address),
    provider_id=42
)

print(f"Group handle created")
print(f"Group view has {group.view.size} members")

# Access group view
for i, member in enumerate(group.view.members):
    print(f"  Member {i}: {member.address} (provider_id={member.provider_id})")

# Cleanup
del group
del flock_client
del provider
engine.finalize()

print("\nQuickstart completed successfully!")
