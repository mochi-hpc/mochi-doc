add_executable (06_ssg_join_proc1 proc1.c)
target_link_libraries (06_ssg_join_proc1 PkgConfig::SSG)

add_executable (06_ssg_join_proc2 proc2.c)
target_link_libraries (06_ssg_join_proc2 PkgConfig::SSG)
