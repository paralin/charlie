#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <protogen/manager.pb.h>

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

    private:
      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;
      CManagerConfig config;
    };
  };
};
