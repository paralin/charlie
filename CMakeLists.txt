project(charlie)

cmake_minimum_required(VERSION 3.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CHARLIE_MODULE_PATH "${CMAKE_SOURCE_DIR}/src/modules/")
set(CHARLIE_SERVER_MODULE_PATH "${CMAKE_SOURCE_DIR}/src/server_modules/")

# Standard files included in every module
set(CHARLIE_MODULE_SRC
  "${CMAKE_SOURCE_DIR}/src/networking/emsgsizes.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/Crypto.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/CryptoBuf.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/xor.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/base64.cpp"
  "charlie.pb.redacted.cc"
  "charlie_net.pb.redacted.cc"
  "client.pb.redacted.cc")

# Standard linking
set(CHARLIE_MODULE_LINK manager)

add_definitions("-Wno-deprecated-declarations")
add_definitions("-Wno-builtin-macro-redefined")
if(WINCC)
  #Undo override by the toolchain
  MESSAGE( STATUS "Overriding build type ${CMAKE_BUILD_TYPE} with ${ACTUAL_BUILD_TYPE}...")
  set(CMAKE_BUILD_TYPE ${ACTUAL_BUILD_TYPE})
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions("-DDO_CHARLIE_LOG -DDEBUG")
endif()

find_package(DL REQUIRED)
find_package(Threads REQUIRED)
set(THREAD_LIB ${CMAKE_THREAD_LIBS_INIT})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} "so")
set(STATIC_LIBC_ARGS "-static-libgcc -static-libstdc++")

if(NOT WINCC)
  message( STATUS "Not using Boost system paths." )
  set(Boost_NO_SYSTEM_PATHS ON)
  set(BOOST_ROOT "${PROJECT_SOURCE_DIR}/deps/boost/final/")
endif()

find_package(Boost REQUIRED COMPONENTS filesystem system program_options regex thread chrono)

if(WINCC)
  find_package(Protobuf REQUIRED)
else()
  set(Protobuf_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/deps/protobuf/final/include/")
  set(PROTOBUF_LITE_LIBRARIES "${PROJECT_SOURCE_DIR}/deps/protobuf/final/lib/libprotobuf-lite.a")
  set(PROTOBUF_LIBRARIES "${PROJECT_SOURCE_DIR}/deps/protobuf/final/lib/libprotobuf.a")
endif()

find_package(GLIB REQUIRED COMPONENTS gmodule)
if(WINCC)
  find_package(OpenSSL REQUIRED)
else()
  set(OPENSSL_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/deps/openssl/final/include/")
  set(OPENSSL_LIBRARIES "${PROJECT_SOURCE_DIR}/deps/openssl/final/lib/libssl.a" "${PROJECT_SOURCE_DIR}/deps/openssl/final/lib/libcrypto.a")
  set(OPENSSL_FOUND ON)
endif()

set(PROCESS_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/deps/process/")

if(WINCC)
  find_package(ZLIB REQUIRED)
else()
  set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/deps/zlib/final/)
  add_subdirectory(deps/zlib)
  # Might not be necessary
  set_target_properties(zlibstatic PROPERTIES COMPILE_FLAGS "-fPIC")
  set(ZLIB_LIBRARIES zlibstatic)
  set(ZLIB_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
endif()

# This will not disable https
set(HTTP_ONLY ON CACHE BOOL "")
set(CURL_STATICLIB ON CACHE BOOL "")
set(BUILD_CURL_EXE OFF CACHE BOOL "")
set(BUILD_CURL_TESTS OFF CACHE BOOL "")
set(CURL_ZLIB ON CACHE BOOL "")
#set(CMAKE_USE_LIBSSH2 ON CACHE BOOL "")
set(CMAKE_USE_OPENSSL ON CACHE BOOL "")
set(ENABLE_MANUAL OFF CACHE BOOL "")
#set(CURL_DISABLE_CRYPTO_AUTH ON CACHE BOOL "")
# See #558 on curl
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CURL_DISABLE_VERBOSE_STRINGS OFF CACHE BOOL "")
else()
  set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "")
endif()
add_subdirectory(deps/curl)
if (NOT WINCC)
  set_target_properties(libcurl PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

#Remove pthread from the boost libraries
list(REMOVE_AT Boost_LIBRARIES -1)

set(include_t1 ${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${PROCESS_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS} ${THREADS_PTHREADS_INCLUDE_DIR})
include_directories(${include_t1})
include_directories("include")
include_directories("${PROJECT_SOURCE_DIR}/deps/rapidjson/include/")
include_directories("${PROJECT_SOURCE_DIR}/deps/mongoose/")

MESSAGE( STATUS "GModule: ${GLIB_GMODULE_LIBRARIES}")
MESSAGE( STATUS "GLIB: ${GLIB_LIBRARIES}")
MESSAGE( STATUS "BOOST inc: ${Boost_INCLUDE_DIRS}")
MESSAGE( STATUS "Other inc: ${include_t1}" )

set(CURL_LIBRARIES_DEPS libcurl)
set(LIBS -Wl,-Bstatic ${PROTOBUF_LITE_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${GLIB_GMODULE_LIBRARIES} ${GLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} -Wl,-Bdynamic ${CMAKE_THREAD_LIBS_INIT} ${DL_LIBRARIES})
set(MANAGER_LIBS -Wl,-Bstatic ${CURL_LIBRARIES_DEPS} ${PROTOBUF_LITE_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY} ${OPENSSL_LIBRARIES} -Wl,-Bdynamic ${CMAKE_THREAD_LIBS_INIT} ${DL_LIBRARIES})

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

  add_definitions("-DPTW32_STATIC_LIB -DWIN32_LEAN_AND_MEAN")
  list(APPEND LIBS -Wl,-Bstatic ${WINSOCK_LIBRARY} ${Intl_LIBRARIES} winmm ${IPHLPAP_LIBRARY} ${LIBICONV_LIBRARY})
  list(APPEND MANAGER_LIBS -Wl,-Bstatic ${WINSOCK_LIBRARY} ${Intl_LIBRARIES} winmm ${IPHLPAP_LIBRARY} ${LIBICONV_LIBRARY})
endif()

set(STEALTH_FLAGS "-D__FILE__='\"\"' -D__DIR__='\"\"' -Wno-builtin-macro-redefined -fno-rtti -DBOOST_NO_RTTI -DGOOGLE_PROTOBUF_NO_RTTI -D_WIN32_WINNT=0x0501 -fvisibility=hidden")
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(STEALTH_FLAGS "${STEALTH_FLAGS}")
  set(MODULE_FLAGS "${STEALTH_FLAGS} -DCHARLIE_MODULE")
else()
  set(STEALTH_FLAGS "${STEALTH_FLAGS} -Os -s")
  set(MODULE_FLAGS "${STEALTH_FLAGS} -DCHARLIE_MODULE")
endif()
if (NOT WINCC)
  set(MODULE_FLAGS "${MODULE_FLAGS} -fPIC")
endif()

if(NO_TOOLS)
  add_custom_target(cutils COMMENT "Not compiling tools.")
else()
  # Build compile tool
  add_executable(cutils
    "src/cutils/main.cpp"
    "src/protogen/charlie.pb.cc"
    "src/protogen/charlie_net.pb.cc"
    "src/protogen/manager.pb.cc"
    "src/protogen/client.pb.cc"
    "src/charlie/xor.cpp"
    "src/charlie/Crypto.cpp"
    "src/charlie/hash.cpp"
    "src/cserver/ModuleTable.cpp"
    )
  target_link_libraries(cutils ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_REGEX_LIBRARY} ${LIBS})
endif()

# Redact files
set(redact_proto)
file(GLOB proto_files "src/proto/*.proto")
foreach(file ${proto_files})
  get_filename_component (file_without_extension ${file} NAME_WE)
  list(APPEND redact_proto ${file_without_extension})
endforeach()

MESSAGE( STATUS "Will redact charlie proto files." )
foreach(to_redact IN LISTS redact_proto)
  MESSAGE( STATUS "Planning to redact ${to_redact} proto files..." )
  file(COPY "${CMAKE_SOURCE_DIR}/src/protogen/${to_redact}.pb.h" DESTINATION ${CMAKE_BINARY_DIR})
  add_custom_command(
    OUTPUT ${to_redact}.pb.redacted.cc
    COMMAND "${CMAKE_BINARY_DIR}/cutils"
    ARGS protoclean --input "${CMAKE_SOURCE_DIR}/src/protogen/${to_redact}.pb.cc" --output ${to_redact}.pb.redacted.cc
    DEPENDS cutils
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

file(GLOB module_incs "${CHARLIE_MODULE_PATH}/**/module.cmake")
foreach(file ${module_incs})
  MESSAGE( STATUS "Including module ${file}...")
  include(${file})
endforeach()

file(GLOB server_module_incs "${CHARLIE_SERVER_MODULE_PATH}/**/module.cmake")
foreach(file ${server_module_incs})
  MESSAGE( STATUS "Including server module ${file}...")
  include(${file})
endforeach()

# Set the list of libraries to compile embed code
set(embed_libraries manager)

# Copy server identity file
if(EXISTS "${PROJECT_SOURCE_DIR}/resources/identity/server_identity")
  file(COPY "${PROJECT_SOURCE_DIR}/resources/identity/server_identity" DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/server_identity")
  MESSAGE( STATUS "Server identity exists, encoding it into server_pubkey" )
  add_custom_target(server_identity COMMENT "Using existing server identity.")
else()
  MESSAGE( STATUS "Server identity doesn't exist, one will be re-generated..." )
  add_custom_command(
    OUTPUT server_identity
    COMMAND "${CMAKE_BINARY_DIR}/cutils"
    ARGS genident --output server_identity --xorkey serveridentity
    DEPENDS cutils
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
MESSAGE( STATUS "The server identity will be encoded into a pubkey..." )
add_custom_command(
  OUTPUT server_pubkey
  COMMAND "${CMAKE_BINARY_DIR}/cutils"
  ARGS genpubkey --identity server_identity --output server_pubkey --xorkey serveridentity
  DEPENDS cutils server_identity
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

MESSAGE( STATUS "Planning to generate the initial module table..." )
add_custom_command(
  OUTPUT init_mtable
  COMMAND "${CMAKE_BINARY_DIR}/cutils"
  ARGS genmodtable --xorkey 70016b8d0c80f1393f8155f897f7e63b7fc2ef0b45c42763325019ef8983ace0 --output init_mtable --sign --identity server_identity --identxor serveridentity --json ../src/config/initmtable.json
  DEPENDS cutils server_identity manager
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Compile the data into c files (use with plugin_data.c dependency)
foreach(to_embed_lib IN LISTS embed_libraries)
  add_custom_command(
    OUTPUT ${to_embed_lib}_data.c
    COMMAND "${CMAKE_BINARY_DIR}/cutils"
    ARGS embed --compress --output ${to_embed_lib}_data.c --input "${CMAKE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${to_embed_lib}${CMAKE_SHARED_LIBRARY_SUFFIX}" --xorkey GpVDIRK4KfsYC9WGbyXZ
    DEPENDS ${to_embed_lib} cutils
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

# Compile server pubkey into c file
add_custom_command(
  OUTPUT server_pubkey_data.c
  COMMAND "${CMAKE_BINARY_DIR}/cutils"
  ARGS embed --output server_pubkey_data.c --input "${CMAKE_BINARY_DIR}/server_pubkey"
  DEPENDS server_pubkey cutils
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Compile initial module table into c file
add_custom_command(
  OUTPUT init_modtable_data.c
  COMMAND "${CMAKE_BINARY_DIR}/cutils"
  ARGS embed --output init_modtable_data.c --input "${CMAKE_BINARY_DIR}/init_mtable"
  DEPENDS init_mtable cutils
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Build charlie
file(GLOB charlie_SRC "src/charlie/*.cpp")
add_executable(charlie manager_data.c server_pubkey_data.c init_modtable_data.c ${charlie_SRC} "charlie.pb.redacted.cc")
set_target_properties(charlie PROPERTIES COMPILE_FLAGS "${STEALTH_FLAGS}")
set_target_properties(charlie PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(charlie ${LIBS})

# Now let's compile the server
if(NOT WINCC)
  # Compile mongoose
  add_library(mongoose STATIC "${PROJECT_SOURCE_DIR}/deps/mongoose/mongoose.c")
  set_target_properties(mongoose PROPERTIES COMPILE_FLAGS "${STEALTH_FLAGS}")

  set(SERVER_LIBS -Wl,-Bstatic mongoose ${CURL_LIBRARIES_DEPS} ${OPENSSL_LIBRARIES} ${PROTOBUF_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_CHRONO_LIBRARY} ${GLIB_GMODULE_LIBRARIES} ${GLIB_LIBRARIES}  -Wl,-Bdynamic ${CMAKE_THREAD_LIBS_INIT} ${DL_LIBRARIES})

  file(GLOB cserver_SRC "src/cserver/*.cpp")
  file(GLOB protogen_SRC "src/protogen/*.cc")
  file(GLOB http_SRC "src/http/*.cpp")
  add_executable(cserver ${cserver_SRC} ${protogen_SRC} ${http_SRC}
    "src/networking/emsgsizes.cpp"
    "src/charlie/Crypto.cpp"
    "src/charlie/CryptoBuf.cpp"
    "src/charlie/base64.cpp"
    "src/charlie/hash.cpp"
    "src/charlie/xor.cpp"
    "src/charlie/smear.cpp"
    )
  target_link_libraries(cserver ${SERVER_LIBS})
  set_target_properties(cserver PROPERTIES COMPILE_FLAGS "${STEALTH_FLAGS} -DDO_CHARLIE_LOG")
  set_target_properties(cserver PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
 endif()
