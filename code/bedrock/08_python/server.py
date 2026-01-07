#!/usr/bin/env python
"""
Example of starting a Bedrock service from Python.
"""
from mochi.bedrock.server import Server
import time

# Configuration as a Python dictionary
config = {
    "margo": {
        "argobots": {
            "pools": [
                {"name": "my_pool", "kind": "fifo_wait", "access": "mpmc"}
            ]
        }
    },
    "libraries": [
        "libyokan-bedrock-module.so",
        "libflock-bedrock-module.so"
    ],
    "providers": [
        {
            "name": "my_database",
            "type": "yokan",
            "provider_id": 42,
            "config": {
                "database": {
                    "type": "map"
                }
            },
            "dependencies": {
                "pool": "__primary__"
            }
        },
        {
            "name": "my_group",
            "type": "flock",
            "provider_id": 33,
            "config": {
                "bootstrap": "self",
                "group": {
                    "type": "static",
                    "config": {}
                },
                "file": "mygroup.flock"
            },
            "dependencies": {
                "pool": "__primary__"
            }
        }
    ]
}

# Start the Bedrock server
server = Server("na+sm", config=config)

print(f"Bedrock server started at {server.margo.engine.address}")

server.wait_for_finalize()
print("Server finalized")
