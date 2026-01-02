cmake_minimum_required(VERSION 3.10)
project(yokan-cpp-examples CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(yokan REQUIRED)
find_package(thallium REQUIRED)

set(EXAMPLES
    quickstart
    client_database
    basic_operations
    batch_operations
    modes
    exceptions
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} yokan::client thallium)
endforeach()
