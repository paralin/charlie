# Build manager (server version)
file(GLOB_RECURSE s_manager_SRC "${CHARLIE_SERVER_MODULE_PATH}/manager/*.cpp")
add_library(s_manager SHARED ${s_manager_SRC} ${CHARLIE_MODULE_SRC}
  "manager.pb.redacted.cc")
set_target_properties(s_manager PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DCHARLIE_MODULE_NAME='\"manager\"'")
set_target_properties(s_manager PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(s_manager -Wl,-Bstatic ${PROTOBUF_LITE_LIBRARIES})
add_custom_command(TARGET manager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:s_manager> ${CMAKE_BINARY_DIR}/server_modules/${CMAKE_SHARED_LIBRARY_PREFIX}manager${CMAKE_SHARED_LIBRARY_SUFFIX}
)
