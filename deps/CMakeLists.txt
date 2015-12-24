project(charlie_deps)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/final/)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")

add_definitions("-Wno-deprecated-declarations")
add_definitions("-DPTW32_STATIC_LIB -DWIN32_LEAN_AND_MEAN")
add_definitions("-Wno-builtin-macro-redefined")

set(THREAD_LIB ${CMAKE_THREAD_LIBS_INIT})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} "so")
set(STATIC_LIBC_ARGS "-static-libgcc -static-libstdc++")

if(NOT WINCC)
  message( STATUS "Not using Boost system paths." )
  set(Boost_NO_SYSTEM_PATHS ON)
  set(BOOST_ROOT "${PROJECT_SOURCE_DIR}/boost/final/")
endif()

find_package(Boost REQUIRED COMPONENTS filesystem system program_options regex thread chrono)
find_package(GLIB REQUIRED COMPONENTS gmodule)
find_package(Threads REQUIRED)
if(WINCC)
  find_package(OpenSSL REQUIRED)
else()
  set(OPENSSL_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/openssl/final/include/")
  set(OPENSSL_LIBRARIES "${PROJECT_SOURCE_DIR}/openssl/final/lib/libssl.a" "${PROJECT_SOURCE_DIR}/openssl/final/lib/libcrypto.a")
  set(OPENSSL_FOUND ON)
endif()

set(PROCESS_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/process/")

add_subdirectory(zlib)
set_target_properties(zlibstatic PROPERTIES COMPILE_FLAGS "-fPIC")

# This will not disable https
set(HTTP_ONLY ON CACHE BOOL "")
set(CURL_STATICLIB ON CACHE BOOL "")
set(BUILD_CURL_EXE OFF CACHE BOOL "")
set(BUILD_CURL_TESTS OFF CACHE BOOL "")
set(CURL_ZLIB ON CACHE BOOL "")
set(CMAKE_USE_OPENSSL ON CACHE BOOL "")
set(ENABLE_MANUAL OFF CACHE BOOL "")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CURL_DISABLE_VERBOSE_STRINGS OFF CACHE BOOL "")
else()
  set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "")
endif()
add_subdirectory(curl)
if (NOT WINCC)
  set_target_properties(libcurl PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

#Remove pthread from the boost libraries
list(REMOVE_AT Boost_LIBRARIES -1)
include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${PROCESS_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS} ${THREADS_PTHREADS_INCLUDE_DIR})
set(CURL_LIBRARIES_DEPS libcurl)
set(LIBS -Wl,-Bstatic ${PROTOBUF_LITE_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${GLIB_GMODULE_LIBRARIES} ${GLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} -Wl,-Bdynamic ${CMAKE_THREAD_LIBS_INIT} ${DL_LIBRARIES})

if(WINCC)
  find_package(Libiconv REQUIRED)
  find_library(WINSOCK_LIBRARY NAMES ws2_32
    PATHS /lib
    /usr/lib
    /usr/local/lib)
  find_library(IPHLPAP_LIBRARY NAMES iphlpapi
    PATHS /lib
    /usr/lib
    /usr/local/lib)
  find_package(Intl REQUIRED)
endif()

set(EVENT__DISABLE_SAMPLES ON)
set(EVENT__DISABLE_TESTS ON)
set(EVENT__DISABLE_REGRESS ON)
set(EVENT__DISABLE_BENCHMARK ON)
add_subdirectory(libevent)
set_target_properties(event PROPERTIES COMPILE_FLAGS "-fPIC")
