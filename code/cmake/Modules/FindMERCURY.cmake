# - Try to find MERCURY
# Once done, this will define
#
#  MERCURY_FOUND - system has MERCURY
#  MERCURY_INCLUDE_DIRS - the MERCURY include directories
#  MERCURY_LIBRARIES - link these to use MERCURY

# Include dir
find_path(MERCURY_INCLUDE_DIR
  mercury.h
  HINTS ${MERCURY_ROOT}/include
)

# Finally the library itself
find_library(MERCURY_LIBRARY 
  NAMES
  mercury
  mercury_debug
  HINTS ${MERCURY_ROOT}/lib
)

find_library(MERCURY_HL_LIBRARY
  NAMES
  mercury_hl
  mercury_hl_debug
  HINTS ${MERCURY_ROOT}/lib
)

find_library(MERCURY_UTIL_LIBRARY
  NAMES
  mercury_util
  HINTS ${MERCURY_ROOT}/lib
)

set(MERCURY_LIBRARIES ${MERCURY_LIBRARY} ${MERCURY_HL_LIBRARY} ${MERCURY_UTIL_LIBRARY})
set(MERCURY_INCLUDE_DIRS ${MERCURY_INCLUDE_DIR} ${MERCURY_INCLUDE_DIR}/mercury_util)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MERCURY_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(MERCURY  DEFAULT_MSG
                                  MERCURY_LIBRARY MERCURY_INCLUDE_DIR)

mark_as_advanced(MERCURY_INCLUDE_DIR MERCURY_LIBRARY)
