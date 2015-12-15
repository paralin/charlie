#pragma once
#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>
#include "DirectConnectInter.h"
#include <ModuleNet.h>
#include <set>
#include <modules/manager/ManagerInter.h>

#include <boost/thread/mutex.hpp>

namespace modules
{
  namespace directconnect
  {
    class DirectConnectModule : public modules::Module
    {
    public:
      DirectConnectModule();
      ~DirectConnectModule();

      void shutdown();
      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);
      void* getPublicInterface();
      void handleEvent(u32 event, void* data);
      void module_main();

    private:
      ModuleInterface* mInter;
      DirectConnectInter *pInter;
      modules::manager::ManagerInter* manager;
      std::set<std::string> serverKeys;
      boost::mutex managerMtx;
      std::set<std::string> server;

      void populateServerKeys();
    };
  };
};
