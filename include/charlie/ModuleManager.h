#pragma once

#include <Common.h>
#include <Logging.h>
#include <Module.h>

//#include <charlie/System.h>
#include <charlie/ModuleInstance.h>
#include <charlie/SystemInfo.h>

#include <protogen/charlie.pb.h>
#include <iostream>
#include <functional>
#include <string>
#include <memory>

//Forward declaration
class System;
class ModuleManager {
public:
  ModuleManager(System* sys);
  ~ModuleManager();

  void setSystemInfo(SystemInfo* info);
  bool parseModuleTable(charlie::CSignedBuffer* buf, charlie::CModuleTable* outp);

private:
  std::map <uint32_t, ModuleInstance> modules;
  System* sys;
  SystemInfo* sysInfo;
};
