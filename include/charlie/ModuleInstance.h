#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <protogen/charlie.pb.h>
#include <string>
#include <glib.h>
#include <gmodule.h>

typedef modules::Module* (*ConstructFunc) (void);
class ModuleInstance
{
public:
  ModuleInstance(charlie::CModule* mod, std::string path);
  ~ModuleInstance();

  bool load();
  void unload();

  charlie::CModuleInstance inst;

private:
  charlie::CModule* module;
  std::string libPath;
  inline void setStatus(::charlie::EModuleStatus value);
  modules::Module* baseModule;
  GModule* gmod;
};
