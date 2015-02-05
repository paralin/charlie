#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <protogen/charlie.pb.h>
#include <string>
#include <glib.h>
#include <gmodule.h>
#include <charlie/ModuleInterImpl.h>
#include <set>
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
  std::set<u32> modReqs;
  void notifyModuleReleased(u32 id);

  charlie::CModule* module;

private:
  std::string libPath;
  inline void setStatus(::charlie::EModuleStatus value);
  modules::Module* baseModule;
  GModule* gmod;
  ModuleManager* mManager;
  ModuleInterImpl * mInter;
  boost::thread* mainThread;
};
