cmake_minimum_required(VERSION 3.10)
project(flock-intro C)

find_package(flock-server REQUIRED)
find_package(flock-client REQUIRED)

add_executable(server server.c)
target_link_libraries(server flock-server flock-client)

add_executable(client client.c)
target_link_libraries(client flock-server flock-client)
