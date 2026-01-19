"""
Working with GroupView objects.
"""

from mochi.flock.view import GroupView

print("=== GroupView Operations ===\n")

# Create empty group view
view = GroupView()
print(f"Created a view with {len(view.members)} members and {len(view.metadata)} metadata")

# Add members
view.members.add("tcp://127.0.0.1:1234", 1)
view.members.add("tcp://127.0.0.1:1235", 2)
view.members.add("tcp://127.0.0.1:1236", 3)

print(f"\nAfter adding 3 members, size: {len(view.members)}")

# Iterate over members
print("\nMembers in view:")
for i, member in enumerate(view.members):
    print(f"  {i}: {member.address} (provider_id={member.provider_id})")

# Access specific member by index
print(f"\nFirst member: {view.members[0].address}")
print(f"Second member: {view.members[1].address}")

# Digest (hash) of view
digest = view.digest
print(f"\nView digest (hash): {digest}")

# Metadata
view.metadata.add("application", "my_app")
view.metadata.add("version", "1.0")

# Access metadata
for i, m in enumerate(view.metadata):
    print(f" - metadata {i}: {m.key} => {m.value}")

print("\nGroupView operations completed!")
