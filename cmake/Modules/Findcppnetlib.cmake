
# Assume not found.
SET(CPPNETLIB_FOUND FALSE)

# PATH ________________________________________________________________________

if(NOT CPPNETLIB_PATH)
    find_path(CPPNETLIB_PATH include/boost/network.hpp
      HINTS ${CPPNETLIB_ROOT} $ENV{CPPNETLIB_ROOT}
      /usr/local/
      /usr/
       /
    )
endif ()

# HEADERS AND DYNAMIC LIBRARIES_________________________________________________

if(CPPNETLIB_PATH)
    set (CPPNETLIB_INCLUDE_DIR ${CPPNETLIB_PATH}/include)
    mark_as_advanced (CPPNETLIB_INCLUDE_DIR)

    find_library(CPPNETLIB_LIBRARY1 NAMES cppnetlib-client-connections
      HINTS "${CPPNETLIB_PATH}/../../" ${CPPNETLIB_ROOT} $ENV{CPPNETLIB_ROOT}
        PATHS ${CPPNETLIB_PATH}/lib64 ${CPPNETLIB_PATH}/lib
    )
    find_library(CPPNETLIB_LIBRARY2 NAMES cppnetlib-server-parsers
        HINTS "${CPPNETLIB_PATH}/../../" ${CPPNETLIB_ROOT} $ENV{CPPNETLIB_ROOT}
        PATHS ${CPPNETLIB_PATH}/lib64 ${CPPNETLIB_PATH}/lib
    )
    find_library(CPPNETLIB_LIBRARY3 NAMES cppnetlib-uri
        HINTS "${CPPNETLIB_PATH}/../../" ${CPPNETLIB_ROOT} $ENV{CPPNETLIB_ROOT}
        PATHS ${CPPNETLIB_PATH}/lib64 ${CPPNETLIB_PATH}/lib
    )
    SET(CPPNETLIB_LIBRARIES ${CPPNETLIB_LIBRARY1} ${CPPNETLIB_LIBRARY2} ${CPPNETLIB_LIBRARY3})
    mark_as_advanced(CPPNETLIB_LIBRARIES)
endif()

# FOUND _______________________________________________________________________
if(CPPNETLIB_FIND_REQUIRED)
    set(_cppnetlib_output 1)
else()
    if(NOT CPPNETLIB_FIND_QUIETLY)
        set(_cppnetlib_output 1)
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CPPNETLIB DEFAULT_MSG CPPNETLIB_LIBRARIES CPPNETLIB_INCLUDE_DIR)

set(CPPNETLIB_INCLUDE_DIRS ${CPPNETLIB_INCLUDE_DIR})

if(CPPNETLIB_FOUND AND _cppnetlib_output )
    message(STATUS "Found cpp-netlib in ${CPPNETLIB_INCLUDE_DIR};${CPPNETLIB_LIBRARIES}")
endif()
