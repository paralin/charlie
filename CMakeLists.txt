cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(charlie)

set(CMAKE_BUILD_TYPE Debug)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(Protobuf REQUIRED)
find_package(GLIB REQUIRED COMPONENTS gmodule)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS})
include_directories("include")

set(LIBS ${Boost_LIBRARIES} ${GLIB_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} ${OPENSSL_LIBRARIES} ${PROTOBUF_LIBRARIES} dl)
set(CFLAGS "-std=c++0x")

file(GLOB proto_SRC "src/proto/*.cc")
file(GLOB server_proto_SRC "src/server_proto/*.cc")

# Build server identity tool
add_executable(genidentity
  "src/sidentity/main.cpp"
  "src/charlie/Crypto.cpp"
  "src/proto/charlie.pb.cc"
  "src/charlie/xor.cpp"
  )
target_link_libraries(genidentity ${LIBS})

# Build embed tool
add_executable(embed "src/embed/main.cpp")
target_link_libraries(embed ${LIBS})

# Build test plugin
file(GLOB testplugin_SRC "src/testplugin/*.cpp")
add_library(testplugin SHARED ${testplugin_SRC})

# Set the list of libraries to compile embed code
set(embed_libraries testplugin)

# Copy server identity file

if(EXISTS "${PROJECT_SOURCE_DIR}/resources/identity/server_identity")
  file(COPY "${PROJECT_SOURCE_DIR}/resources/identity/server_identity" DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/server_identity")
  MESSAGE( STATUS "Server identity exists, encoding it into server_pubkey" )
else()
  MESSAGE( STATUS "Server identity doesn't exist, one will be re-generated..." )
  add_custom_command(
    OUTPUT server_identity
    COMMAND genidentity server_identity serveridentity
    DEPENDS genidentity
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
MESSAGE( STATUS "The server identity will be encoded into a pubkey..." )
add_custom_command(
  OUTPUT server_pubkey
  COMMAND genidentity
  ARGS -finishCMake server_identity server_pubkey serveridentity
  DEPENDS genidentity server_identity
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Compile the data into c files (use with plugin_data.c dependency)
foreach(to_embed_lib IN LISTS embed_libraries)
  add_custom_command(
    OUTPUT ${to_embed_lib}_data.c
    COMMAND embed ${to_embed_lib}_data "${CMAKE_BINARY_DIR}/${to_embed_lib}"
    DEPENDS ${to_embed_lib} embed
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

# Compile server pubkey into c file
add_custom_command(
  OUTPUT server_pubkey_data.c
  COMMAND embed server_pubkey_data "${CMAKE_BINARY_DIR}/server_pubkey" -nomodule
  DEPENDS server_pubkey embed
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

# Build charlie
file(GLOB charlie_SRC "src/charlie/*.cpp")
add_executable(charlie testplugin_data.c server_pubkey_data.c ${charlie_SRC} ${proto_SRC})

#add_dependencies(charlie embed)
target_link_libraries(charlie ${LIBS})
