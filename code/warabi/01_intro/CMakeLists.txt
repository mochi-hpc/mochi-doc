cmake_minimum_required(VERSION 3.10)
project(warabi-intro CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(warabi REQUIRED)

add_executable(01_warabi_server server.cpp)
target_link_libraries(01_warabi_server warabi::server warabi::client)

add_executable(01_warabi_client client.cpp)
target_link_libraries(01_warabi_client warabi::server warabi::client)
