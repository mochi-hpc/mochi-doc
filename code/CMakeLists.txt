#
# CMakeLists.txt  top-level cmake file for mochi-doc
# 25-Avr-2019  mdorier@anl.gov
#
cmake_minimum_required (VERSION 3.12)
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
# pkg-config, for non-cmake dependencies
find_package (PkgConfig REQUIRED)
# MPI and PMIx (both optional) for SSG
find_package(MPI)
find_package(PMIx)
# mercury
find_package (mercury REQUIRED)
#include_directories(${MERCURY_INCLUDE_DIR})
# thallium
find_package (thallium REQUIRED)
# abt-io
pkg_check_modules (ABTIO REQUIRED IMPORTED_TARGET abt-io)
# ssg
pkg_check_modules (SSG REQUIRED IMPORTED_TARGET ssg)
# margo
pkg_check_modules (MARGO REQUIRED IMPORTED_TARGET margo)
# argobots
pkg_check_modules (ABT REQUIRED IMPORTED_TARGET argobots)

add_subdirectory (argobots)
add_subdirectory (mercury)
add_subdirectory (margo)
add_subdirectory (thallium)
add_subdirectory (abtio)
add_subdirectory (ssg)
