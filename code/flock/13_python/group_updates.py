"""
Updating group membership (for centralized backend).
"""

import mochi.flock.server as server
import mochi.flock.client as client
from mochi.flock.view import GroupView
import mochi.margo
import json

print("=== Group Updates Example ===\n")

# Setup server with centralized backend (allows updates)
engine = mochi.margo.Engine("na+sm", mochi.margo.server)

initial_view = GroupView()
initial_view.members.add(str(engine.address), 42)

config = {
    "group": {
        "type": "centralized",
        "config": {}
    }
}

provider = server.Provider(
    engine=engine,
    provider_id=42,
    config=json.dumps(config),
    initial_view=initial_view
)

print(f"Centralized backend provider created")

# Create client and group handle
flock_client = client.Client(engine)
group = flock_client.make_group_handle(
    address=str(engine.address),
    provider_id=42
)

print(f"Initial group size: {len(group.view.members)}")
for i, member in enumerate(group.view.members):
    print(f"  Member {i}: {member.address}")

# Update group view (fetch latest from server)
print("\nCalling update() to refresh view...")
group.update()

print(f"After update, group size: {len(group.view.members)}")

# Note: In a real distributed scenario, other processes would join/leave
# and update() would reflect those changes

# Access view properties
print(f"\nGroup view digest: {group.view.digest}")
print(f"View has metadata: {len(group.view.metadata)} entries")

# Cleanup
engine.finalize()

print("\nGroup updates example completed!")
