add_executable (02_abtio_main main.c)
target_link_libraries (02_abtio_main PkgConfig::ABTIO PkgConfig::ABT)
