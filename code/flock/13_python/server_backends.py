"""
Examples of different Flock backend configurations.
"""

import mochi.flock.server as server
from mochi.flock.view import GroupView
import mochi.margo
import json

def static_backend_example():
    """Static backend: membership is fixed at initialization."""
    print("=== Static Backend Example ===")

    engine = mochi.margo.Engine("na+sm", mochi.margo.server)

    # Create initial view with multiple members
    initial_view = GroupView()
    initial_view.members.add(str(engine.address), 42)
    print(f"Initial view size: {len(initial_view.members)}")
    # Could add more members if we had multiple processes

    config = {
        "group": {
            "type": "static",
            "config": {}
        }
    }

    provider = server.Provider(
        engine=engine,
        provider_id=42,
        config=json.dumps(config),
        initial_view=initial_view
    )

    print(f"Static backend provider created")

    engine.finalize()


def centralized_backend_example():
    """Centralized backend: one server manages membership."""
    print("\n=== Centralized Backend Example ===")

    engine = mochi.margo.Engine("na+sm", mochi.margo.server)

    initial_view = GroupView()
    initial_view.members.add(str(engine.address), 43)

    config = {
        "group": {
            "type": "centralized",
            "config": {
            }
        }
    }

    provider = server.Provider(
        engine=engine,
        provider_id=43,
        config=json.dumps(config),
        initial_view=initial_view
    )

    print(f"Centralized backend provider created")
    print("Centralized backend allows dynamic membership changes")

    engine.finalize()


if __name__ == "__main__":
    static_backend_example()
    centralized_backend_example()
    print("\nBackend examples completed!")
