set(TOR_ROOT_DIR "${CMAKE_SOURCE_DIR}/deps/tor/")
set(TOR_EXT_DIR "${TOR_ROOT_DIR}/src/ext/")
include_directories(${TOR_ROOT_DIR})
include_directories(${TOR_ROOT_DIR}/src)
include_directories(${TOR_ROOT_DIR}/src/common)
include_directories(${TOR_ROOT_DIR}/src/or)
include_directories(${TOR_ROOT_DIR}/src/ext)
include_directories(${TOR_ROOT_DIR}/src/ext/trunnel)
include_directories(${TOR_ROOT_DIR}/src/trunnel)
include_directories(AFTER ${M_INCLUDES} ${EVENT2_INCLUDES})

if (WINCC)
  set(tor_thread_SRC
    "${TOR_ROOT_DIR}/src/common/compat_winthreads.c"
    )
else()
  set(tor_thread_SRC
    "${TOR_ROOT_DIR}/src/common/compat_pthreads.c"
    )
endif()

set(tor_common_SRC
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
  "${TOR_ROOT_DIR}/src/common/aes.c"
  "${TOR_ROOT_DIR}/src/common/procmon.c"
  "${TOR_ROOT_DIR}/src/common/util_format.c"
  "${TOR_ROOT_DIR}/src/common/torgzip.c"
  "${TOR_ROOT_DIR}/src/common/workqueue.c"
  "${TOR_ROOT_DIR}/src/common/tortls.c"
  "${TOR_ROOT_DIR}/src/common/crypto_format.c"
  "${TOR_ROOT_DIR}/src/common/crypto_format.c"
  "${TOR_ROOT_DIR}/src/common/crypto_curve25519.c"
  "${TOR_ROOT_DIR}/src/common/crypto_ed25519.c"
  "${TOR_ROOT_DIR}/src/common/crypto_s2k.c"
  "${TOR_ROOT_DIR}/src/common/crypto_pwbox.c"
  "${TOR_ROOT_DIR}/src/common/compat_threads.c"
  ${tor_thread_SRC}
  "${TOR_ROOT_DIR}/src/common/compat_libevent.c")

file(GLOB_RECURSE tor_or_SRC "${TOR_ROOT_DIR}/src/or/*.c")
list(REMOVE_ITEM tor_or_SRC "${TOR_ROOT_DIR}/src/or/main.c")
list(REMOVE_ITEM tor_or_SRC "${TOR_ROOT_DIR}/src/or/ntmain.c")
list(REMOVE_ITEM tor_or_SRC "${TOR_ROOT_DIR}/src/or/tor_main.c")
set(tor_trunnel_SRC
  "${TOR_ROOT_DIR}/src/trunnel/link_handshake.c"
  "${TOR_ROOT_DIR}/src/trunnel/ed25519_cert.c"
  "${TOR_ROOT_DIR}/src/trunnel/pwbox.c"
  )
file(GLOB tor_ref10_SRC "${TOR_ROOT_DIR}/src/ext/ed25519/ref10/*.c")
set(tor_ext_SRC
  ${tor_ref10_SRC}
  "${TOR_EXT_DIR}/csiphash.c"
  "${TOR_EXT_DIR}/trunnel/trunnel.c"
  "${TOR_EXT_DIR}/ed25519/donna/ed25519_tor.c"
  )
if (WINCC)
  set(tor_ext_SRC ${tor_ext_SRC} "${TOR_EXT_DIR}/curve25519_donna/curve25519-donna.c")
else()
  set(tor_ext_SRC ${tor_ext_SRC} "${TOR_EXT_DIR}/curve25519_donna/curve25519-donna-c64.c")
endif()
set(tor_all_SRC ${tor_or_SRC} ${tor_ext_SRC} ${tor_common_SRC} ${tor_trunnel_SRC})

# Build torm
file(GLOB_RECURSE torm_SRC "${CHARLIE_MODULE_PATH}/torm/*.cpp")
add_library(torm SHARED ${torm_SRC} ${CHARLIE_MODULE_SRC}
  "${CHARLIE_MODULE_PATH}/torm/torc.c"
  "${CMAKE_SOURCE_DIR}/src/charlie/hash.cpp"
  "${CMAKE_SOURCE_DIR}/src/charlie/random.cpp"
  "${CMAKE_SOURCE_DIR}/src/networking/CharlieSession.cpp"
  "${CMAKE_SOURCE_DIR}/src/networking/emsgsizes.cpp"
  ${tor_all_SRC}
  "torm.pb.redacted.cc"
  )
set_target_properties(torm PROPERTIES COMPILE_FLAGS "${MODULE_FLAGS} -std=gnu99 -DCHARLIE_MODULE_NAME='\"tor\"' -DIS_CHARLIE -fPIC -DED25519_CUSTOMRANDOM -DED25519_SUFFIX=_donna -D_POSIX")
set_target_properties(torm PROPERTIES CMAKE_BUILD_TYPE Debug)
set_target_properties(torm PROPERTIES LINK_FLAGS     ${STATIC_LIBC_ARGS})
target_link_libraries(torm manager client ${CHARLIE_MODULE_LINK} ${PROTOBUF_LITE_LIBRARIES} ${OPENSSL_LIBRARIES} ${EVENT2_LIBRARIES} ${GLIB_LIBRARIES} ${M_LIBRARIES} ${RT_LIBRARIES} client)
add_custom_command(TARGET torm POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:torm> ${CMAKE_SHARED_LIBRARY_PREFIX}6032034${CMAKE_SHARED_LIBRARY_SUFFIX}
)
