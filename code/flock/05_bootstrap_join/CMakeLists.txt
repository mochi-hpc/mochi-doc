cmake_minimum_required (VERSION 3.10)
project (flock-bootstrap-join C)


find_package (flock REQUIRED)

add_executable (05_flock_server server.c)
target_link_libraries(05_flock_server flock::server flock::client)
