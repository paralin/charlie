cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(charlie)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS thread system log)
find_package(Protobuf REQUIRED)
find_package(GLIB REQUIRED COMPONENTS gmodule)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS})
    include_directories("include")

    set(LIBS ${Boost_LIBRARIES} ${GLIB_LIBRARIES} ${GLIB_GMODULE_LIBRARIES} dl)
    set(CFLAGS "-std=c++0x")

    # Build charlie
    file(GLOB charlie_SRC "src/charlie/*.cpp")
    add_executable(charlie ${charlie_SRC})
    target_link_libraries(charlie ${LIBS})

    # Build
    file(GLOB testplugin_SRC "src/testplugin/*.cpp")
    add_library(testplugin SHARED ${testplugin_SRC})
else()
    message( FATAL_ERROR "Missing some requirements, check the cmake log." )
endif()
