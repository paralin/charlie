#pragma once
#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>
#include "ClientInter.h"
#include <modules/manager/ManagerInter.h>
#include <ModuleNet.h>

#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>

namespace modules
{
  namespace client
  {
    class ClientModule : public modules::Module
    {
    public:
      ClientModule();
      ~ClientModule();

      void shutdown();
      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);
      void* getPublicInterface();
      void handleEvent(u32 event, void* data);
      void module_main();

    private:
      void selectNetworkModule();

      modules::manager::ManagerInter* manager;
      boost::mutex managerInterMtx;
      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;
      ClientInter *pInter;

      modules::ModuleNet* netModule;
      u32 netModuleId;
    };
  };
};
