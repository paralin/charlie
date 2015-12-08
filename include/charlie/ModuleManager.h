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
#include <boost/thread/mutex.hpp>

//Forward declaration
class System;
class ModuleManager {
public:
  ModuleManager(System* sys);
  ~ModuleManager();

  System* sys;
  SystemInfo* sysInfo;

  void setSystemInfo(SystemInfo* info);
  bool parseModuleTable(charlie::CSignedBuffer* buf, charlie::CModuleTable* outp);

  //Check if the manager lib exists; if not return false
  bool moduleLoadable(charlie::CModule* mod, bool cleanFail=false);
  bool moduleLoadable(u32 id, bool cleanFail=false);
  char* getModuleFilename(charlie::CModule* mod);
  charlie::CModule* findModule(u32 id, int*idx=NULL);
  bool moduleRunning(u32 id);

  //Deferred actions
  void deferRecheckModules();
  void deferSaveConfig();
  void deferReloadModule(u32 id);

  //Main loop
  void updateEverything();

  //Top level requirements
  std::set<u32> tlReqs;

  //Internal stuff
  void onModuleReleased(u32 id);

  //Verify new module table
  bool loadIncomingModuleTable(charlie::CSignedBuffer* buf);

  //Get storage for a module
  charlie::CModuleStorage* storageForModule(u32 id);

  ModuleInstance* getModuleInstance(u32 id);

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
  std::set<u32> toReload;

  //Check first loop run
  bool modulesDirty;
  bool configDirty;

  boost::mutex mtx;
  bool hasShutdown = false;
};
