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
#include <set>

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
  bool moduleRunning(u32 id);

  //Don't call these directly
  int launchModule(charlie::CModule* mod);
  int launchModule(int id);

  //Top level requirements
  std::set<u32> tlReqs;

  //Merge a final array of needed modules
  void evaluateRequirements();

private:
  std::map <int, std::shared_ptr<ModuleInstance>> minstances;
  System* sys;
  SystemInfo* sysInfo;
};
