cmake_minimum_required(VERSION 3.10)
project(flock-client-api C)


find_package(flock-client REQUIRED)

add_executable(client client.c)
target_link_libraries(client flock-client)
