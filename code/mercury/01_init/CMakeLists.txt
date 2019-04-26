add_executable(01_hg_server server.c)
target_link_libraries(01_hg_server mercury)

add_executable(01_hg_client client.c)
target_link_libraries(01_hg_client mercury)
