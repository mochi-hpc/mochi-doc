cmake_minimum_required(VERSION 3.10)
project(warabi-backends-memory CXX)

set(CMAKE_CXX_STANDARD 14)


find_package(warabi REQUIRED)

add_executable(03_warabi_server server.cpp)
target_link_libraries(03_warabi_server warabi::server)
