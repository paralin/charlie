#pragma once
#include <IntTypes.h>
#include <protogen/charlie.pb.h>

class ModuleManager;
class ModuleInstance;
namespace modules
{
  class ModuleInterface
  {
    public:
      ModuleInterface(ModuleManager *manager, ModuleInstance * inst);
      ~ModuleInterface();

      //Request that a module be loaded
      int  requireDependency(u32 id);
      void releaseDependency(u32 id);

    private:
      ModuleManager *mManager;
      ModuleInstance * inst;
  };
};
