#pragma once

#include <Common.h>
#include <Logging.h>
#include <Module.h>
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <charlie/ModuleInstance.h>

class ModuleManager {
public:
  ModuleManager();
  ~ModuleManager();

private:
  std::map <uint32_t, ModuleInstance> modules;
};
