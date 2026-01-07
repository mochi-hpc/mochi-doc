cmake_minimum_required (VERSION 3.10)
project (yokan-migration C)

find_package (PkgConfig REQUIRED)
pkg_check_modules (MARGO REQUIRED IMPORTED_TARGET margo)
find_package (remi REQUIRED)
find_package (yokan REQUIRED)

add_executable (migrate_example migrate_example.c)
target_link_libraries (migrate_example
    PkgConfig::MARGO
    remi
    yokan::server
    yokan::client)
