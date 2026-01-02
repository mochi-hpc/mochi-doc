cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-04 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (04_abt_basic_scheduler basic_scheduler.c)
target_link_libraries (04_abt_basic_scheduler PkgConfig::ABT)

add_executable (04_abt_priority_scheduler priority_scheduler.c)
target_link_libraries (04_abt_priority_scheduler PkgConfig::ABT)

add_executable (04_abt_work_stealing work_stealing.c)
target_link_libraries (04_abt_work_stealing PkgConfig::ABT)

add_executable (04_abt_fibonacci fibonacci.c)
target_link_libraries (04_abt_fibonacci PkgConfig::ABT)
