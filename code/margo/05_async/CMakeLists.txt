add_executable (05_margo_server server.c)
target_link_libraries (05_margo_server PkgConfig::MARGO)

add_executable (05_margo_client client.c)
target_link_libraries (05_margo_client PkgConfig::MARGO)
