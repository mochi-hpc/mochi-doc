cmake_minimum_required(VERSION 3.10)
project(flock-cpp CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(flock-server REQUIRED)
find_package(flock-client REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server flock-server flock-client)

add_executable(client client.cpp)
target_link_libraries(client flock-server flock-client)
