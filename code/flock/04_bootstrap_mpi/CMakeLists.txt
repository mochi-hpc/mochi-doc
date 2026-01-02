cmake_minimum_required(VERSION 3.10)
project(flock-bootstrap-mpi C)


find_package(MPI REQUIRED)

find_package(flock-server REQUIRED)

add_executable(server server.c)
target_link_libraries(server flock-server MPI::MPI_C)
