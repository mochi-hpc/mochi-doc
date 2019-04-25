# - Try to find Argobots 
# Once done this will define
#  MARGO_FOUND - System has Argobots
#  MARGO_INCLUDE_DIRS - The Argobots include directories
#  MARGO_LIBRARIES - The libraries needed to use Argobots
#  MARGO_DEFINITIONS - Compiler switches required for using Argobots

find_package(PkgConfig)
pkg_check_modules(PC_MARGO QUIET libxml-2.0)
set(MARGO_DEFINITIONS ${PC_MARGO_CFLAGS_OTHER})

find_path(MARGO_INCLUDE_DIR margo.h
          HINTS ${PC_MARGO_INCLUDEDIR} ${PC_MARGO_INCLUDE_DIRS} ${MARGO_ROOT}/include)

find_library(MARGO_LIBRARY NAMES margo libmargo
             HINTS ${PC_MARGO_LIBDIR} ${PC_MARGO_LIBRARY_DIRS} ${MARGO_ROOT}/lib)

set(MARGO_LIBRARIES ${MARGO_LIBRARY} )
set(MARGO_INCLUDE_DIRS ${MARGO_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MARGO2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(MARGO  DEFAULT_MSG
                                  MARGO_LIBRARY MARGO_INCLUDE_DIR)

mark_as_advanced(MARGO_INCLUDE_DIR MARGO_LIBRARY)
