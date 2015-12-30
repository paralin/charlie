# Build manager
file(GLOB manager_SRC "${CHARLIE_MODULE_PATH}/manager/*.cpp")
add_library(manager SHARED ${manager_SRC} ${CHARLIE_MODULE_SRC} "manager.pb.redacted.cc"
  "${CMAKE_SOURCE_DIR}/src/charlie/machine_id.cpp")
set_target_properties(manager PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DCHARLIE_MODULE_NAME='\"manager\"'")
set_target_properties(manager PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(manager ${MANAGER_LIBS})
add_custom_command(TARGET manager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:manager> ${CMAKE_SHARED_LIBRARY_PREFIX}3133916783${CMAKE_SHARED_LIBRARY_SUFFIX}
)
