#include <Module.h>
#include <Logging.h>

namespace modules
{
  namespace manager
  {
    class ManagerModule : public modules::Module
    {
    public:
      ManagerModule()
      {
        CLOG("Constructing manager module...");
      }

      //Called when the module is about to be deleted
      void shutdown()
      {
        CLOG("Deconstructing manager module..");
      }

      int injectDependency(u32 id, void* dep)
      {
      };
    };
  };
};

CHARLIE_CONSTRUCT(modules::manager::ManagerModule);
