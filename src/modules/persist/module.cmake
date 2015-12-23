# Build persist
file(GLOB_RECURSE persist_SRC "${CMAKE_MODULE_PATH}/persist/*.cpp")
add_library(persist SHARED ${persist_SRC} ${CHARLIE_MODULE_SRC})
set_target_properties(persist PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DCHARLIE_MODULE_NAME='\"persist\"'")
set_target_properties(persist PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(persist -Wl,-Bstatic  ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${PROTOBUF_LITE_LIBRARIES})
add_custom_command(TARGET persist POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:persist> ${CMAKE_SHARED_LIBRARY_PREFIX}2526948902${CMAKE_SHARED_LIBRARY_SUFFIX}
)
