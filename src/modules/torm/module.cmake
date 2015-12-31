set(TOR_ROOT_DIR "${CMAKE_SOURCE_DIR}/deps/tor/")
include_directories(${TOR_ROOT_DIR})
include_directories(${TOR_ROOT_DIR}/src)
include_directories(${TOR_ROOT_DIR}/src/common)
include_directories(${TOR_ROOT_DIR}/src/or)
include_directories(${TOR_ROOT_DIR}/src/ext)
include_directories(${TOR_ROOT_DIR}/src/ext/trunnel)
include_directories(${TOR_ROOT_DIR}/src/trunnel)
include_directories(AFTER ${M_INCLUDES} ${EVENT2_INCLUDES})

add_library(tor STATIC IMPORTED)
set_target_properties(tor PROPERTIES IMPORTED_LOCATION "${TOR_ROOT_DIR}/src/or/${CMAKE_STATIC_LIBRARY_PREFIX}tor${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(tor-crypto STATIC IMPORTED)
set_target_properties(tor-crypto PROPERTIES IMPORTED_LOCATION "${TOR_ROOT_DIR}/src/common/${CMAKE_STATIC_LIBRARY_PREFIX}or-crypto${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(tor-event STATIC IMPORTED)
set_target_properties(tor-event PROPERTIES IMPORTED_LOCATION "${TOR_ROOT_DIR}/src/common/${CMAKE_STATIC_LIBRARY_PREFIX}or-event${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(tor-trunnel STATIC IMPORTED)
set_target_properties(tor-trunnel PROPERTIES IMPORTED_LOCATION "${TOR_ROOT_DIR}/src/trunnel/${CMAKE_STATIC_LIBRARY_PREFIX}or-trunnel${CMAKE_STATIC_LIBRARY_SUFFIX}")

file(GLOB_RECURSE torext_LIBS "${TOR_ROOT_DIR}/src/ext/${CMAKE_STATIC_LIBRARY_PREFIX}*${CMAKE_STATIC_LIBRARY_SUFFIX}")

# Build torm
file(GLOB_RECURSE torm_SRC "${CHARLIE_MODULE_PATH}/torm/*.cpp")
file(GLOB tor_common_SRC "${TOR_ROOT_DIR}/src/common/*.c")
add_library(torm SHARED ${torm_SRC} ${CHARLIE_MODULE_SRC} "torm.pb.redacted.cc" "${CHARLIE_MODULE_PATH}/torm/torc.c"
  "${CMAKE_SOURCE_DIR}/src/charlie/hash.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/random.cpp"
  "${CMAKE_SOURCE_DIR}/src/networking/CharlieSession.cpp"
  "${CMAKE_SOURCE_DIR}/src/networking/emsgsizes.cpp"
  "${TOR_ROOT_DIR}/src/common/compat.c"
  "${TOR_ROOT_DIR}/src/common/util.c"
  "${TOR_ROOT_DIR}/src/common/container.c"
  "${TOR_ROOT_DIR}/src/common/backtrace.c"
  "${TOR_ROOT_DIR}/src/common/crypto.c"
  "${TOR_ROOT_DIR}/src/common/address.c"
  "${TOR_ROOT_DIR}/src/common/log.c"
  "${TOR_ROOT_DIR}/src/common/sandbox.c"
  "${TOR_ROOT_DIR}/src/common/memarea.c"
  "${TOR_ROOT_DIR}/src/common/di_ops.c"
  "${TOR_ROOT_DIR}/src/common/util_format.c"
  "${TOR_ROOT_DIR}/src/common/crypto_curve25519.c"
  "${TOR_ROOT_DIR}/src/common/compat_threads.c"
  "${TOR_ROOT_DIR}/src/common/compat_pthreads.c"
  "${TOR_ROOT_DIR}/src/common/compat_winthreads.c"
  "${TOR_ROOT_DIR}/src/common/compat_libevent.c"
  "${TOR_ROOT_DIR}/src/trunnel/link_handshake.c"
  "${TOR_ROOT_DIR}/src/ext/csiphash.c"
  "${TOR_ROOT_DIR}/src/ext/curve25519_donna/curve25519-donna-c64.c"
  )
set_target_properties(torm PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -DCHARLIE_MODULE_NAME='\"tor\"' -DIS_CHARLIE -fPIC")
set_target_properties(torm PROPERTIES CMAKE_BUILD_TYPE Debug)
set_target_properties(torm PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(torm manager client ${CHARLIE_MODULE_LINK} -Wl,-Bstatic tor tor-crypto tor-event tor-trunnel ${torext_LIBS} ${PROTOBUF_LITE_LIBRARIES} ${OPENSSL_LIBRARIES} ${EVENT2_LIBRARIES} ${GLIB_LIBRARIES} ${M_LIBRARIES} ${RT_LIBRARIES})
add_custom_command(TARGET torm POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:torm> ${CMAKE_SHARED_LIBRARY_PREFIX}6032034${CMAKE_SHARED_LIBRARY_SUFFIX}
)
