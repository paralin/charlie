#pragma once

class ModuleManager;

namespace modules
{
  class ModuleInterface
  {
    public:
      ModuleInterface(ModuleManager *manager);
      ~ModuleInterface();

      int requireDependency(u32 id);
    private:
      ModuleManager *manager;
  };
};
