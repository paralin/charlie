#pragma once
#ifndef TORC_H
#define TORC_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"

  void torc_main(int bindPort, const char* socksUsername, const char* socksPassword, void* torminst);
  void torc_shutdown();
  void torc_new_identity();
  int have_completed_a_circuit();
  void torc_serialize_data(void* torm, const char* torfname, const char* data, size_t len, int flags);
  void torc_delete_data(void* torm, const char* torfname);
  file_status_t torc_stat_data(void* torm, const char* torfname);
  char* torc_read_data(void* torm, const char* torfname, int isBinary);
  file_status_t file_status(const char* fname);

#ifdef __cplusplus
}
#endif

#endif
