add_executable (07_ssg_observe_proc1 proc1.c)
target_link_libraries (07_ssg_observe_proc1 PkgConfig::SSG)

add_executable (07_ssg_observe_proc2 proc2.c)
target_link_libraries (07_ssg_observe_proc2 PkgConfig::SSG)
