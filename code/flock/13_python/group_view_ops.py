"""
Working with GroupView objects.
"""

from mochi.flock.view import GroupView

print("=== GroupView Operations ===\n")

# Create empty group view
view = GroupView()
print(f"Created empty view, size: {view.size}")

# Add members
view.members.add("tcp://127.0.0.1:1234", 1)
view.members.add("tcp://127.0.0.1:1235", 2)
view.members.add("tcp://127.0.0.1:1236", 3)

print(f"\nAfter adding 3 members, size: {view.size}")

# Iterate over members
print("\nMembers in view:")
for i, member in enumerate(view.members):
    print(f"  {i}: {member.address} (provider_id={member.provider_id})")

# Access specific member by index
print(f"\nFirst member: {view.members[0].address}")
print(f"Second member: {view.members[1].address}")

# Get member rank by address
rank = view.members.find("tcp://127.0.0.1:1235", 2)
print(f"\nRank of tcp://127.0.0.1:1235 (provider_id=2): {rank}")

# Check membership
is_member = view.members.contains("tcp://127.0.0.1:1234", 1)
print(f"Contains tcp://127.0.0.1:1234 (provider_id=1): {is_member}")

# Digest (hash) of view
digest = view.digest
print(f"\nView digest (hash): {digest}")

# Metadata
view.metadata["application"] = "my_app"
view.metadata["version"] = "1.0"
print(f"\nMetadata: {dict(view.metadata)}")

print("\nGroupView operations completed!")
