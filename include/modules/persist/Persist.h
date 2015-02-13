#include <Common.h>
#include <Module.h>
#include <Logging.h>
//#include <protogen/manager.pb.h>

namespace modules
{
  namespace persist
  {
    class PersistModule : public modules::Module
    {
    public:
      //Only constructor. Destructor doesn't work.
      PersistModule();
      void shutdown();

      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);

      void* getPublicInterface();

    private:
      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;

      //Satisfy the compiler
      void module_main(){}
    };
  };
};
