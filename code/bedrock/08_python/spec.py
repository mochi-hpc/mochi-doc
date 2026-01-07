from mochi.bedrock.spec import (
        ProcSpec,
        PoolSpec,
        XstreamSpec,
        ProviderSpec,
        SchedulerSpec,
        MargoSpec
)
from mochi.bedrock.server import Server

# Create pool specifications
pool1 = PoolSpec(name="pool1", kind="fifo_wait", access="mpmc")
pool2 = PoolSpec(name="pool2", kind="fifo_wait", access="mpmc")

# Create xstream specifications
xstream1 = XstreamSpec(
    name="xstream1",
    scheduler=SchedulerSpec(
        type="basic_wait",
        pools=[pool1, pool2]
    )
)

# Create provider specification
provider = ProviderSpec(
    name="my_provider",
    type="yokan",
    provider_id=42,
    dependencies={
        "pool": "pool1"
    },
    config={"database": {"type": "map"}}
)

# Build complete process specification
spec = ProcSpec(margo="na+sm")

# Convert to dictionary or JSON
config_dict = spec.to_dict()
config_json = spec.to_json()

# Start server with this configuration
server = Server("na+sm", config=spec)
server.wait_for_finalize()
