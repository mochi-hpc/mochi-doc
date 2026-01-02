cmake_minimum_required(VERSION 3.10)
project(yokan-migration C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MARGO REQUIRED IMPORTED_TARGET margo)
find_package(yokan REQUIRED)

set(EXAMPLES
    migrate_basic
    migrate_remove
    migrate_filter
    migrate_bedrock
    migrate_error
    migrate_incremental
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.c)
    target_link_libraries(${example} PkgConfig::MARGO yokan::client)
endforeach()
