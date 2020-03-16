if (MPI_FOUND)
  add_executable (04_ssg_create_mpi main.c)
  target_link_libraries (04_ssg_create_mpi ssg margo)
endif (MPI_FOUND)
