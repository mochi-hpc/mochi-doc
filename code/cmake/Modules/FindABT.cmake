# - Try to find Argobots 
# Once done this will define
#  ABT_FOUND - System has Argobots
#  ABT_INCLUDE_DIRS - The Argobots include directories
#  ABT_LIBRARIES - The libraries needed to use Argobots
#  ABT_DEFINITIONS - Compiler switches required for using Argobots

find_package(PkgConfig)
pkg_check_modules(PC_ABT QUIET libxml-2.0)
set(ABT_DEFINITIONS ${PC_ABT_CFLAGS_OTHER})

find_path(ABT_INCLUDE_DIR abt.h
          HINTS ${PC_ABT_INCLUDEDIR} ${PC_ABT_INCLUDE_DIRS} ${ABT_ROOT}/include)

find_library(ABT_LIBRARY NAMES abt libabt
             HINTS ${PC_ABT_LIBDIR} ${PC_ABT_LIBRARY_DIRS} ${ABT_ROOT}/lib)

set(ABT_LIBRARIES ${ABT_LIBRARY} )
set(ABT_INCLUDE_DIRS ${ABT_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ABT2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ABT  DEFAULT_MSG
                                  ABT_LIBRARY ABT_INCLUDE_DIR)

mark_as_advanced(ABT_INCLUDE_DIR ABT_LIBRARY)
