cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(charlie)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED COMPONENTS filesystem system program_options regex thread chrono)
find_package(Protobuf REQUIRED)
find_package(GLIB REQUIRED COMPONENTS gmodule)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS})
include_directories("include")
include_directories("${PROJECT_SOURCE_DIR}/deps/rapidjson/include/")

set(LIBS ${Boost_LIBRARIES} ${GLIB_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} ${OPENSSL_LIBRARIES} ${PROTOBUF_LITE_LIBRARIES} dl)
# add_definitions("-std=c++11 -D__FILE__ \"file\"")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(STEALTH_FLAGS "-fno-rtti -DGOOGLE_PROTOBUF_NO_RTTI -Os -fvisibility=hidden -s")
set(MODULE_FLAGS "${STEALTH_FLAGS} -DCHARLIE_MODULE")

# Build compile tool
add_executable(cutils
  "src/cutils/main.cpp"
  "src/protogen/charlie.pb.cc"
  "src/charlie/xor.cpp"
  "src/charlie/Crypto.cpp"
  "src/charlie/hash.cpp"
  )
target_link_libraries(cutils ${LIBS})

# Redact files
set(proto_SRC charlie.pb.redacted.cc)

set(REDACT_CH_PTH "${PROJECT_SOURCE_DIR}/src/protogen/charlie.pb.cc")
if(EXISTS ${REDACT_CH_PTH})
  MESSAGE( STATUS "Will redact charlie proto files." )
  file(COPY "${PROJECT_SOURCE_DIR}/src/protogen/charlie.pb.h" DESTINATION ${CMAKE_BINARY_DIR})
  add_custom_command(
    OUTPUT charlie.pb.redacted.cc
    COMMAND cutils
    ARGS protoclean --input ${REDACT_CH_PTH} --output charlie.pb.redacted.cc
    DEPENDS cutils
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Build manager
file(GLOB manager_SRC "src/modules/manager/*.cpp" ${proto_SRC})
add_library(manager SHARED ${manager_SRC})
set_target_properties(manager PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS}")

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
    COMMAND cutils
    ARGS genident --output server_identity --xorkey serveridentity
    DEPENDS cutils
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
MESSAGE( STATUS "The server identity will be encoded into a pubkey..." )
add_custom_command(
  OUTPUT server_pubkey
  COMMAND cutils
  ARGS genpubkey --identity server_identity --output server_pubkey --xorkey serveridentity
  DEPENDS cutils server_identity
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

MESSAGE( STATUS "Planning to generate the initial module table..." )
add_custom_command(
  OUTPUT init_mtable
  COMMAND cutils
  ARGS genmodtable --xorkey 70016b8d0c80f1393f8155f897f7e63b7fc2ef0b45c42763325019ef8983ace0 --output init_mtable --sign --hash --identity server_identity --identxor serveridentity --json ../src/config/initmtable.json
  DEPENDS cutils server_identity
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Compile the data into c files (use with plugin_data.c dependency)
foreach(to_embed_lib IN LISTS embed_libraries)
  add_custom_command(
    OUTPUT ${to_embed_lib}_data.c
    COMMAND cutils
    ARGS embed --output ${to_embed_lib}_data.c --input "${CMAKE_BINARY_DIR}/${to_embed_lib}" --xorkey GpVDIRK4KfsYC9WGbyXZ --usegmodule
    DEPENDS ${to_embed_lib} cutils
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

# Compile server pubkey into c file
add_custom_command(
  OUTPUT server_pubkey_data.c
  COMMAND cutils
  ARGS embed --output server_pubkey_data.c --input "${CMAKE_BINARY_DIR}/server_pubkey"
  DEPENDS server_pubkey cutils
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Compile initial module table into c file
add_custom_command(
  OUTPUT init_modtable_data.c
  COMMAND cutils
  ARGS embed --output init_modtable_data.c --input "${CMAKE_BINARY_DIR}/init_mtable"
  DEPENDS init_mtable cutils
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Build charlie
file(GLOB charlie_SRC "src/charlie/*.cpp")
add_executable(charlie manager_data.c server_pubkey_data.c init_modtable_data.c ${charlie_SRC} ${proto_SRC})
set_target_properties(charlie PROPERTIES COMPILE_FLAGS "${STEALTH_FLAGS}")

#add_dependencies(charlie embed)
target_link_libraries(charlie ${LIBS})
