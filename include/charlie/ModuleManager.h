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
#include <queue>
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

  //Check if the manager lib exists; if not return false
  bool moduleLoadable(std::shared_ptr<charlie::CModule> mod, bool cleanFail=true);
  bool moduleLoadable(u32 id, bool cleanFail=true);
  char* getModuleFilename(std::shared_ptr<charlie::CModule> mod);
  std::shared_ptr<charlie::CModule> findModule(u32 id, int*idx=NULL);
  static charlie::CModuleBinary* selectBinary(std::shared_ptr<charlie::CModule> mod, int*idx=NULL);
  std::shared_ptr<charlie::CModule> selectModule(std::vector<std::shared_ptr<charlie::CModule>>& mods, charlie::CModuleBinary** bin = NULL);
  std::shared_ptr<charlie::CModule> selectModule(u32 cap, charlie::CModuleBinary** bin = NULL);
  std::vector<std::shared_ptr<charlie::CModule>> listModulesWithCap(u32 cap, bool filterHasBinary);
  bool moduleRunning(u32 id);

  //Deferred actions
  void deferRecheckModules();
  void deferSaveConfig();
  void deferReloadModule(u32 id);

  void statusChanged(u32 id, charlie::EModuleStatus status);

  //Main loop
  void updateEverything();

  //Top level requirements
  std::set<u32> tlReqs;

  //Internal stuff
  void onModuleReleased(u32 id);

  //Verify new module table
  void loadIncomingModuleTable(const charlie::CModuleTable& tab);

  //Get storage for a module
  charlie::CModuleStorage* storageForModule(u32 id);

  ModuleInstance* getModuleInstance(u32 id);

  std::vector<charlie::CModuleInstance> listModuleInstances();

private:
  //Don't call these directly
  int launchModule(std::shared_ptr<charlie::CModule> mod);
  int launchModule(u32 id);
  int launchModuleWithChecks(std::shared_ptr<charlie::CModule> mod);
  int launchModuleWithChecks(u32 id);

  // Events
  void transmitEvent(charlie::EModuleEvents eve, void* data);

  //Merge a final array of needed modules
  void evaluateRequirements();
  std::map<u32, std::shared_ptr<ModuleInstance>> minstances;
  std::set<u32> toReload;
  std::set<u32> pendingLoad;
  std::set<u32> notifyRelease;
  // Depended upon modules
  std::set<u32> dependedUpon;
  std::map<u32, charlie::EModuleStatus> pendingStatusNotify;

  //Check first loop run
  bool modulesDirty;
  bool configDirty;
  bool pendingLoadDirty;
  bool dependedUponDirty;
  bool moduleTableDirty;

  boost::mutex mtx;
  boost::mutex pModStatusMtx;
  bool hasShutdown = false;
};
