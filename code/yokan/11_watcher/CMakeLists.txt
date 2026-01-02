cmake_minimum_required(VERSION 3.10)
project(yokan-watcher C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MARGO REQUIRED IMPORTED_TARGET margo)
find_package(yokan REQUIRED)
pkg_check_modules(ABT REQUIRED IMPORTED_TARGET argobots)

set(EXAMPLES
    wait_notify_basic
    wait_notify_multi
    producer_consumer
    distributed_coord
    without_wait
    wait_timeout
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.c)
    target_link_libraries(${example} PkgConfig::MARGO yokan::client PkgConfig::ABT)
endforeach()
