from mochi.margo import Engine
from mochi.yokan.server import Provider
from mochi.yokan.client import Client
import json

engine = Engine('tcp')
provider = Provider(engine=engine, provider_id=42,
                   config='{"database":{"type":"map"}}')
client = Client(engine=engine)
db = client.make_database_handle(address=engine.addr(), provider_id=42)

# Get a collection handle
coll = db.collection(name="users")

# Store JSON documents
doc1 = {"name": "Alice", "age": 30, "city": "Boston"}
doc2 = {"name": "Bob", "age": 25, "city": "New York"}
doc3 = {"name": "Carol", "age": 35, "city": "Boston"}

# Store documents and get their IDs
id1 = coll.store(doc=json.dumps(doc1))
id2 = coll.store(doc=json.dumps(doc2))
id3 = coll.store(doc=json.dumps(doc3))

print(f"Stored 3 documents with IDs: {id1}, {id2}, {id3}")

# Load a document by ID
loaded_doc = coll.load(id=id1)
print(f"Loaded document: {loaded_doc}")

# List all documents
all_docs = coll.list_docs(from_id=0, count=10)
print(f"\nAll documents:")
for doc_id, doc_data in all_docs:
    print(f"  ID {doc_id}: {json.loads(doc_data)}")

# Update a document
updated_doc = {"name": "Alice", "age": 31, "city": "Boston"}
coll.update(id=id1, doc=json.dumps(updated_doc))
print(f"\nUpdated document {id1}")

# Erase a document
coll.erase(id=id2)
print(f"Erased document {id2}")

# Count documents
count = coll.size()
print(f"Total documents: {count}")

engine.finalize()
