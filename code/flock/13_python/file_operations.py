"""
File-based bootstrapping and serialization examples.
"""

import mochi.flock.server as server
import mochi.flock.client as client
from mochi.flock.view import GroupView
import mochi.margo
import json
import tempfile
import os

print("=== File-Based Operations ===\n")

# Create temporary file for this example
temp_file = tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json')
group_file = temp_file.name
temp_file.close()

try:
    # Setup server
    engine = mochi.margo.Engine("na+sm", mochi.margo.server)

    initial_view = GroupView()
    initial_view.members.add(str(engine.address), 42)

    config = {
        "group": {
            "type": "static",
            "config": {}
        },
        "file": group_file # Flock will write group info to this file
    }

    provider = server.Provider(
        engine=engine,
        provider_id=42,
        config=json.dumps(config),
        initial_view=initial_view
    )

    print(f"Provider created, group info written to: {group_file}")

    # Client can bootstrap from file
    flock_client = client.Client(engine)

    # Method 1: Create group handle from file
    print("\nMethod 1: Loading from file...")
    group = flock_client.make_group_handle_from_file(group_file)
    print(f"Loaded group with {len(group.view.members)} members")

    # Cleanup
    engine.finalize()

finally:
    # Clean up temp file
    if os.path.exists(group_file):
        os.unlink(group_file)
        print(f"\nCleaned up temporary file: {group_file}")

print("\nFile operations completed!")
