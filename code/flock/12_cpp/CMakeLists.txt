cmake_minimum_required (VERSION 3.10)
project (flock-cpp C CXX)

set (CMAKE_CXX_STANDARD 17)

find_package (flock REQUIRED)

add_executable (12_flock_server server.cpp)
target_link_libraries (12_flock_server flock::cxx-headers flock::server)

add_executable (12_flock_client client.cpp)
target_link_libraries (12_flock_client flock::cxx-headers flock::client)
