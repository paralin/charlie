#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <protogen/manager.pb.h>
#include "ManagerInter.h"

namespace modules
{
  namespace manager
  {
    class ManagerModule : public modules::Module
    {
    public:
      //Only constructor. Destructor doesn't work.
      ManagerModule();
      void shutdown();

      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);

      void module_main();

      void* getPublicInterface();

    private:
      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;
      CManagerInfo sInfo;
      ManagerInter* pInter;
    };
  };
};
