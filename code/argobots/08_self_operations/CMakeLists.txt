cmake_minimum_required (VERSION 3.10)
project (argobots-tutorial-09 C)

# Find Argobots using pkg-config
find_package (PkgConfig REQUIRED)
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

# Build examples
add_executable (09_abt_yield_sync yield_sync.c)
target_link_libraries (09_abt_yield_sync PkgConfig::ABT)

add_executable (09_abt_progress_polling progress_polling.c)
target_link_libraries (09_abt_progress_polling PkgConfig::ABT)
