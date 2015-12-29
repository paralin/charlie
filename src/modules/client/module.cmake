# Build client
file(GLOB_RECURSE client_SRC "${CHARLIE_MODULE_PATH}/client/*.cpp")
file(GLOB_RECURSE client_networking_SRC "${CMAKE_SOURCE_DIR}/src/networking/*.cpp")
add_library(client SHARED ${client_SRC} ${CHARLIE_MODULE_SRC} ${client_networking_SRC}
  "${CMAKE_SOURCE_DIR}/src/charlie/hash.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/random.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/machine_id.cpp")
set_target_properties(client PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DCHARLIE_MODULE_NAME='\"client\"'")
set_target_properties(client PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(client ${CHARLIE_MODULE_LINK} -Wl,-Bstatic ${PROTOBUF_LITE_LIBRARIES})
add_custom_command(TARGET client POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:client> ${CMAKE_SHARED_LIBRARY_PREFIX}2777954855${CMAKE_SHARED_LIBRARY_SUFFIX}
)
