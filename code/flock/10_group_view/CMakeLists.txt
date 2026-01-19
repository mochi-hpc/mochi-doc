cmake_minimum_required (VERSION 3.10)
project (flock-group-view C)


find_package (flock REQUIRED)

add_executable (10_flock_client client.c)
target_link_libraries (10_flock_client flock::client)
