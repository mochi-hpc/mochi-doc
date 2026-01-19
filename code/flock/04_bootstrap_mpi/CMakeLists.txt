cmake_minimum_required (VERSION 3.10)
project (flock-bootstrap-mpi C)


find_package (MPI REQUIRED)
find_package (flock REQUIRED)

add_executable (04_flock_server server.c)
target_link_libraries(04_flock_server flock::server MPI::MPI_C)
