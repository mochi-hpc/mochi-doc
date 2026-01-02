cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-05 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (05_abt_stencil_barrier stencil_barrier.c)
target_link_libraries (05_abt_stencil_barrier PkgConfig::ABT)

add_executable (05_abt_iterative_algorithm iterative_algorithm.c)
target_link_libraries (05_abt_iterative_algorithm PkgConfig::ABT)
