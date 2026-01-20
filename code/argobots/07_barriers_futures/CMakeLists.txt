cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-07 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build barrier examples
add_executable (07_abt_stencil_barrier stencil_barrier.c)
target_link_libraries (07_abt_stencil_barrier PkgConfig::ABT)

add_executable (07_abt_iterative_algorithm iterative_algorithm.c)
target_link_libraries (07_abt_iterative_algorithm PkgConfig::ABT)

# Build future examples
add_executable (07_abt_parallel_reduce parallel_reduce.c)
target_link_libraries (07_abt_parallel_reduce PkgConfig::ABT)

add_executable (07_abt_stencil_future stencil_future.c)
target_link_libraries (07_abt_stencil_future PkgConfig::ABT)
