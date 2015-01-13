#pragma once

#include <Common.h>
#include <Module.h>
#include <proto/charlie.pb.h>

class ModuleInstance
{
public:
  ModuleInstance();
  ~ModuleInstance();

private:
  charlie::CModule module;
};
