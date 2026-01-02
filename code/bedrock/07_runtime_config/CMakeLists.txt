cmake_minimum_required(VERSION 3.10)
project(bedrock-runtime-config CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(bedrock REQUIRED)

# Build all examples
set(EXAMPLES
    basic_client
    load_module
    add_pool
    add_xstream
    add_provider
    change_pool
    migrate
    snapshot
    restore
    async
    query
    jx9_query
    complete
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} bedrock::client)
endforeach()
