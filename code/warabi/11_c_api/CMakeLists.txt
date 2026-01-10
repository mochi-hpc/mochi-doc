cmake_minimum_required(VERSION 3.10)
project(warabi-c-api C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MARGO REQUIRED IMPORTED_TARGET margo)
find_package(warabi REQUIRED)

add_executable(11_warabi_client_c client.c)
target_link_libraries(11_warabi_client_c PkgConfig::MARGO warabi::c-client)

add_executable(11_warabi_server_c server.c)
target_link_libraries(11_warabi_server_c PkgConfig::MARGO warabi::c-server)

add_executable(11_warabi_async_c async_example.c)
target_link_libraries(11_warabi_async_c PkgConfig::MARGO warabi::c-client)
