add_executable (01_yokan_server server.c)
target_link_libraries (01_yokan_server yokan-server)

add_executable (01_yokan_admin admin.c)
target_link_libraries (01_yokan_admin yokan-admin)

add_executable (01_yokan_client client.c)
target_link_libraries (01_yokan_client yokan-client)
