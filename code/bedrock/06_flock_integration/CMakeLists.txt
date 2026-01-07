cmake_minimum_required(VERSION 3.10)
project(bedrock-flock-integration CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(thallium REQUIRED)
find_package(flock REQUIRED)

add_executable(06_bedrock_client client.cpp)
target_link_libraries(06_bedrock_client thallium flock::client flock::cxx-headers)
