cmake_minimum_required (VERSION 3.10)
project (flock-backends-static C)


find_package (flock REQUIRED)

add_executable (07_flock_server server.c)
target_link_libraries(07_flock_server flock::server)
