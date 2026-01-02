cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-01 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build hello_world example
add_executable (01_abt_hello_world hello_world.c)
target_link_libraries (01_abt_hello_world PkgConfig::ABT)
