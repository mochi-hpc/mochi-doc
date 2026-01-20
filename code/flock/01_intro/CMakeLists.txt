cmake_minimum_required (VERSION 3.10)
project (flock-intro C)

find_package (flock REQUIRED)

add_executable (01_flock_server server.c)
target_link_libraries (01_flock_server flock::server flock::client)

add_executable (01_flock_client client.c)
target_link_libraries(01_flock_client flock::server flock::client)
