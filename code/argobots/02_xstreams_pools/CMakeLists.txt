cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-02 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build fixed_allocation example
add_executable (02_abt_fixed_allocation fixed_allocation.c)
target_link_libraries (02_abt_fixed_allocation PkgConfig::ABT)

# Build work_stealing example
add_executable (02_abt_work_stealing work_stealing.c)
target_link_libraries (02_abt_work_stealing PkgConfig::ABT)
