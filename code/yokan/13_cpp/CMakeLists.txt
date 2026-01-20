cmake_minimum_required (VERSION 3.10)
project (yokan-cpp-examples CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (yokan REQUIRED)
find_package (thallium REQUIRED)

set (EXAMPLES
    quickstart
    client_database
    basic_operations
    batch_operations
    modes
    exceptions
    string_helpers
    list_operations
    list_packed
    raii
    custom_memory
    binary_data
    templates
    migration
    collections
    with_thallium
)

foreach (example ${EXAMPLES})
    add_executable (13_yokan_${example} ${example}.cpp)
    target_link_libraries (13_yokan_${example} yokan::client thallium)
endforeach ()

# with_bedrock requires bedrock-client
find_package (bedrock REQUIRED)
add_executable (13_yokan_with_bedrock with_bedrock.cpp)
target_link_libraries (13_yokan_with_bedrock yokan::client thallium bedrock::client)
