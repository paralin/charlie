if (NOT WINCC)
  # Build mongo (server version)
  file(GLOB_RECURSE s_mongo_SRC "${CHARLIE_SERVER_MODULE_PATH}/mongo/*.cpp")
  add_library(s_mongo SHARED ${s_mongo_SRC} ${CHARLIE_MODULE_SRC} "client.pb.redacted.cc" "charlie.pb.redacted.cc" "charlie_net.pb.redacted.cc")
  set_target_properties(s_mongo PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DDO_CHARLIE_LOG -DCHARLIE_MODULE_NAME='\"mongo\"'")
  set_target_properties(s_mongo PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
  target_link_libraries(s_mongo -Wl,-Bstatic mongo ${PROTOBUF_LITE_LIBRARIES} ${Boost_REGEX_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_IOSTREAMS_LIBRARY})
  add_custom_command(TARGET s_mongo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:s_mongo> ${CMAKE_BINARY_DIR}/server_modules/${CMAKE_SHARED_LIBRARY_PREFIX}mongo${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
endif()
