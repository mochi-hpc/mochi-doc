cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-03 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build ULT example
add_executable (03_abt_ult_example ult_example.c)
target_link_libraries (03_abt_ult_example PkgConfig::ABT)

# Build simple example
add_executable (03_abt_simple_example simple_example.c)
target_link_libraries (03_abt_simple_example PkgConfig::ABT)

# Build revive example
add_executable (03_abt_revive_example revive_example.c)
target_link_libraries (03_abt_revive_example PkgConfig::ABT)
