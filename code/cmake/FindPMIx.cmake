# - Try to find libpmix
# Once done this will define
#  PMIX_FOUND - System has libpmix
#  PMIX_INCLUDE_DIRS - The libpmix include directories
#  PMIX_LIBRARIES - The libraries needed to use libpmix

FIND_PATH(WITH_PMIx_PREFIX
    NAMES include/pmix.h
)

FIND_LIBRARY(PMIx_LIBRARIES
    NAMES pmix
    HINTS ${WITH_PMIx_PREFIX}/lib
)

FIND_PATH(PMIx_INCLUDE_DIRS
    NAMES pmix.h
    HINTS ${WITH_PMIx_PREFIX}/include
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PMIx DEFAULT_MSG
    PMIx_LIBRARIES
    PMIx_INCLUDE_DIRS
)

# Hide these vars from ccmake GUI
MARK_AS_ADVANCED(
	PMIx_LIBRARIES
	PMIx_INCLUDE_DIRS
)
