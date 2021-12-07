add_executable (07_yokan_client_multi client-multi.c)
target_link_libraries (07_yokan_client_multi yokan-client)

add_executable (07_yokan_client_packed client-packed.c)
target_link_libraries (07_yokan_client_packed yokan-client)
