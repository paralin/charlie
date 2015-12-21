#pragma once
#include <IntTypes.h>
#include <protogen/charlie.pb.h>

struct ModuleStateChange
{
  u32 mod;
  charlie::EModuleStatus state;
};
