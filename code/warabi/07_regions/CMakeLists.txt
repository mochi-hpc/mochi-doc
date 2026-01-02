cmake_minimum_required(VERSION 3.10)
project(warabi-regions CXX)

set(CMAKE_CXX_STANDARD 14)


find_package(warabi REQUIRED)

add_executable(07_warabi_client client.cpp)
target_link_libraries(07_warabi_client warabi::client)
