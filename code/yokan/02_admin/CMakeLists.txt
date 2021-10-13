add_executable (02_yokan_server server.c)
target_link_libraries (02_yokan_server yokan-server)

add_executable (02_yokan_admin admin.c)
target_link_libraries (02_yokan_admin yokan-admin)
