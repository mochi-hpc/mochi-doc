from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client
import json

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Create a collection
db.create_collection(name="users")

# Check that the collection exists
exists = db.collection_exists(name="users")
print(f"Collection 'users' exists: {exists}")

# Get a collection handle
coll = db.open_collection(name="users")

# Store JSON documents
doc1 = {"name": "Alice", "age": 30, "city": "Boston"}
doc2 = {"name": "Bob", "age": 25, "city": "New York"}
doc3 = {"name": "Carol", "age": 35, "city": "Boston"}

# Store documents and get their IDs
id1 = coll.store(document=json.dumps(doc1))
id2 = coll.store(document=json.dumps(doc2))
id3 = coll.store(document=json.dumps(doc3))

print(f"Stored 3 documents with IDs: {id1}, {id2}, {id3}")

# Load a document by ID
loaded_doc = bytearray(64)
length = coll.load(id=id1, buffer=loaded_doc)
print(f"Loaded document: {loaded_doc[:length].decode()}")

# Update a document
updated_doc = {"name": "Alice", "age": 31, "city": "Boston"}
coll.update(id=id1, document=json.dumps(updated_doc))
print(f"\nUpdated document {id1}")

# List all documents
buffers = [bytearray(64) for i in range(3)]
doc_lengths = coll.list_docs(start_id=0, buffers=buffers)
print(doc_lengths)
print(f"\nAll documents:")
for (i, l), b in zip(doc_lengths, buffers):
    print(f"{i} - {b[:l]}")

# Erase a document
coll.erase(id=id2)
print(f"Erased document {id2}")

# Count documents
count = coll.size()
print(f"Total documents: {count}")

engine.finalize()
