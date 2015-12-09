#pragma once
#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include "ClientInter.h"
#include <modules/manager/ManagerInter.h>
#include <boost/thread/mutex.hpp>

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

    private:
      modules::manager::ManagerInter* manager;
      boost::mutex managerInterMtx;
      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;
      ClientInter *pInter;

      void module_main();
    };
  };
};
