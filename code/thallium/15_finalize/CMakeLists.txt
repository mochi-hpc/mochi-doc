add_executable(15_tl_server server.cpp)
target_link_libraries(15_tl_server thallium)

add_executable(15_tl_client client.cpp)
target_link_libraries(15_tl_client thallium)
