cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-06 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (06_abt_stencil_future stencil_future.c)
target_link_libraries (06_abt_stencil_future PkgConfig::ABT)

add_executable (06_abt_parallel_reduce parallel_reduce.c)
target_link_libraries (06_abt_parallel_reduce PkgConfig::ABT)
