#include <Module.h>
#include <Logging.h>

//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
#define MLOG(msg) CLOG("["<<MODULE_ID<<"m] "<<msg);
#define MERR(msg) CERR("["<<MODULE_ID<<"m]! "<<msg);

namespace modules
{
  namespace manager
  {
    class ManagerModule : public modules::Module
    {
    public:
      ManagerModule()
      {
        MLOG("Constructing manager module...");
      }

      //Called when the module is about to be deleted
      void shutdown()
      {
        MLOG("Deconstructing manager module..");
      }

      void setModuleInterface(ModuleInterface* inter)
      {
        MLOG("Received module interface");
        mInter = inter;
      }

      int injectDependency(u32 id, void* dep)
      {
      };

    private:
      ModuleInterface* mInter;
    };
  };
};

CHARLIE_CONSTRUCT(modules::manager::ManagerModule);
