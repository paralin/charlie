cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(charlie)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS thread system log filesystem)
find_package(Protobuf REQUIRED)
find_package(GLIB REQUIRED COMPONENTS gmodule)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS})
    include_directories("include")

    set(LIBS ${Boost_LIBRARIES} ${GLIB_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} dl)
    set(CFLAGS "-std=c++0x")

    # Build embed tool
    add_executable(embed "src/embed/main.cpp")
    target_link_libraries(embed ${LIBS})

    # Build test plugin
    file(GLOB testplugin_SRC "src/testplugin/*.cpp")
    add_library(testplugin SHARED ${testplugin_SRC})

    # Set the list of libraries to compile embed code
    set(embed_libraries testplugin)

    # Compile the data into c files (use with plugin_data.c dependency)
    foreach(to_embed_lib IN LISTS embed_libraries)
      add_custom_command(
          OUTPUT ${to_embed_lib}_data.c
          COMMAND embed ${to_embed_lib}_data "${CMAKE_BINARY_DIR}/${to_embed_lib}"
          DEPENDS ${to_embed_lib} embed
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          )
    endforeach()

    # Build charlie
    file(GLOB charlie_SRC "src/charlie/*.cpp")
    add_executable(charlie testplugin_data.c ${charlie_SRC})
    #add_dependencies(charlie embed)
    target_link_libraries(charlie ${LIBS})

else()
    message( FATAL_ERROR "Missing some requirements, check the cmake log." )
endif()
