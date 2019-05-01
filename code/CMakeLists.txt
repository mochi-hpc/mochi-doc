#
# CMakeLists.txt  top-level cmake file for mochi-doc
# 25-Avr-2019  mdorier@anl.gov
#
cmake_minimum_required (VERSION 3.1)
project (mochi-doc C CXX)

set (CMAKE_CXX_STANDARD 14)
enable_testing ()

# add our cmake module directory to the path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# link shared lib with full rpath
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# setup cache variables for ccmake
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release
         CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS 
                  "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif ()
set (CMAKE_PREFIX_PATH "" CACHE STRING "External dependencies path")
set (BUILD_SHARED_LIBS "OFF" CACHE BOOL "Build a shared library")

# packages we depend on
include (xpkg-import)
find_package (mercury REQUIRED)
include_directories(${MERCURY_INCLUDE_DIR})
xpkg_import_module (argobots REQUIRED argobots)
xpkg_import_module (margo REQUIRED margo)
find_package (thallium REQUIRED)
xpkg_import_module (abtio REQUIRED abt-io)

add_subdirectory (argobots)
add_subdirectory (mercury)
add_subdirectory (margo)
add_subdirectory (thallium)
add_subdirectory (abtio)
