#pragma once

#include <Common.h>
#include <Module.h>
#include <protogen/charlie.pb.h>

class ModuleInstance
{
public:
  ModuleInstance();
  ~ModuleInstance();

private:
  charlie::CModule module;
};
