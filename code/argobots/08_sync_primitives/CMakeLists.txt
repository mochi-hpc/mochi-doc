cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-08 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (08_abt_eventual_example eventual_example.c)
target_link_libraries (08_abt_eventual_example PkgConfig::ABT)

add_executable (08_abt_rwlock_example rwlock_example.c)
target_link_libraries (08_abt_rwlock_example PkgConfig::ABT)

add_executable (08_abt_thread_local thread_local.c)
target_link_libraries (08_abt_thread_local PkgConfig::ABT)
