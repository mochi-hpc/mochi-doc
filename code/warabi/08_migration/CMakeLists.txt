cmake_minimum_required(VERSION 3.10)
project(warabi-migration CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(warabi REQUIRED)
find_package(remi REQUIRED)

add_executable(08_warabi_migration migration_example.cpp)
target_link_libraries(08_warabi_migration
    warabi::server
    warabi::client
    remi)
