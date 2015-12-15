#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <string>
#include <set>
#include <glib.h>
#include <gmodule.h>

#include <protogen/charlie.pb.h>
#include <charlie/ModuleInterImpl.h>
#include <boost/thread.hpp>

typedef modules::Module* (*ConstructFunc) (void);
class ModuleManager;
class ModuleInstance
{
public:
  ModuleInstance(charlie::CModule* mod, std::string path, ModuleManager* man);
  ~ModuleInstance();

  bool load();
  void unload();

  charlie::CModuleInstance inst;
  // Requirements
  std::set<u32> modReqs;
  // Optional requirements
  std::set<u32> modOptReqs;
  void notifyModuleReleased(u32 id);
  void notifyModuleLoaded(u32 mod, void* ptr);
  charlie::EModuleStatus status();
  void transmitEvent(charlie::EModuleEvents eve, void* data);

  charlie::CModule* module;
  void* publicInterface;

private:
  std::string libPath;
  inline void setStatus(::charlie::EModuleStatus value);
  modules::Module* baseModule;
  GModule* gmod;
  ModuleManager* mManager;
  ModuleInterImpl * mInter;
  boost::thread* mainThread;
};
