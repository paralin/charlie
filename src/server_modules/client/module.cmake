# Build client (server version)
file(GLOB_RECURSE s_client_SRC "${CHARLIE_SERVER_MODULE_PATH}/client/*.cpp")
add_library(s_client SHARED ${s_client_SRC} ${CHARLIE_MODULE_SRC}
  "client.pb.redacted.cc")
set_target_properties(s_client PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DDO_CHARLIE_LOG -DCHARLIE_MODULE_NAME='\"client\"'")
set_target_properties(s_client PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(s_client -Wl,-Bstatic ${PROTOBUF_LITE_LIBRARIES})
add_custom_command(TARGET client POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:s_client> ${CMAKE_BINARY_DIR}/server_modules/${CMAKE_SHARED_LIBRARY_PREFIX}client${CMAKE_SHARED_LIBRARY_SUFFIX}
)
