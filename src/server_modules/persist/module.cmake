# Build persist (server version)
file(GLOB_RECURSE s_persist_SRC "${CHARLIE_SERVER_MODULE_PATH}/persist/*.cpp")
add_library(s_persist SHARED ${s_persist_SRC} ${CHARLIE_MODULE_SRC}
  "persist.pb.redacted.cc")
set_target_properties(s_persist PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DDO_CHARLIE_LOG -DCHARLIE_MODULE_NAME='\"persist\"'")
set_target_properties(s_persist PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(s_persist -Wl,-Bstatic ${PROTOBUF_LITE_LIBRARIES})
add_custom_command(TARGET persist POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:s_persist> ${CMAKE_BINARY_DIR}/server_modules/${CMAKE_SHARED_LIBRARY_PREFIX}persist${CMAKE_SHARED_LIBRARY_SUFFIX}
)
