cmake_minimum_required (VERSION 3.10)
project (flock-backends-centralized C)


find_package (flock REQUIRED)

add_executable (08_flock_server server.c)
target_link_libraries(08_flock_server flock::server)
