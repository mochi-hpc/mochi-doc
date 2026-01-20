cmake_minimum_required (VERSION 3.10)
project (flock-bootstrap-view C)


find_package (flock REQUIRED)

add_executable (03_flock_server server.c)
target_link_libraries (03_flock_server flock::server)
