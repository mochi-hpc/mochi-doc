cmake_minimum_required (VERSION 3.10)
project (flock-bootstrap-file C)


find_package (flock REQUIRED)

add_executable (06_flock_server server.c)
target_link_libraries (06_flock_server flock::server)
