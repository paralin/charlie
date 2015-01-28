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
  bool moduleLoadable(charlie::CModule* mod, bool cleanFail=false);
  char* getModuleFilename(charlie::CModule* mod);
  charlie::CModule* findModule(u32 id, int*idx=NULL);
  bool moduleRunning(u32 id);

  //Set dirty
  void deferRecheckModules();

  //Main loop
  void updateEverything();

  //Top level requirements
  std::set<u32> tlReqs;

  //Internal stuff
  void onModuleReleased(u32 id);

private:
  //Don't call these directly
  int launchModule(charlie::CModule* mod);
  int launchModule(u32 id);
  int launchModuleWithChecks(charlie::CModule* mod);
  int launchModuleWithChecks(u32 id);

  //Merge a final array of needed modules
  void evaluateRequirements();
  std::map <int, std::shared_ptr<ModuleInstance>> minstances;
  std::set<u32> notifyRelease;
  System* sys;
  SystemInfo* sysInfo;

  //Check first loop run
  bool modulesDirty;
};
