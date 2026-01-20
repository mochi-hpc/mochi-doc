cmake_minimum_required(VERSION 3.10)
project(warabi-backends-pmem CXX)

set(CMAKE_CXX_STANDARD 14)


find_package(warabi REQUIRED)

add_executable(04_warabi_server server.cpp)
target_link_libraries(04_warabi_server warabi::server)
