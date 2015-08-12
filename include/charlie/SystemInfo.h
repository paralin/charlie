#pragma once
#include <IntTypes.h>

struct SystemInfo {
  const char* system_id;
  const char* b64_system_id;
  u16         cpu_hash;
  const char* config_filename;
  const char* exe_path;
  const char* root_path;
  u16         lock_port;
};
