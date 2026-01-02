cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-11 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (11_abt_timing_example timing_example.c)
target_link_libraries (11_abt_timing_example PkgConfig::ABT)

add_executable (11_abt_debugging_example debugging_example.c)
target_link_libraries (11_abt_debugging_example PkgConfig::ABT)

add_executable (11_abt_error_handling error_handling.c)
target_link_libraries (11_abt_error_handling PkgConfig::ABT)
