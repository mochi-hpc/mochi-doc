cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-10 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (10_abt_custom_scheduler custom_scheduler.c)
target_link_libraries (10_abt_custom_scheduler PkgConfig::ABT)

add_executable (10_abt_simple_custom simple_custom.c)
target_link_libraries (10_abt_simple_custom PkgConfig::ABT)
