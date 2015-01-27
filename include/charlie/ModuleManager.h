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

  //Check if the manager lib exists; if not return false
  bool moduleLoadable(charlie::CModule* mod);
  char* getModuleFilename(charlie::CModule* mod);
  charlie::CModule* findModule(int id, int*idx=NULL);
  int launchModule(charlie::CModule* mod);
  int launchModule(int id);
  bool moduleRunning(int id);

private:
  std::map <int, std::shared_ptr<ModuleInstance>> minstances;
  System* sys;
  SystemInfo* sysInfo;
};
