#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>

#include "TormInter.h"

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <protogen/torm.pb.h>
#include <boost/thread.hpp>

namespace modules
{
  namespace torm
  {
    class TormModule : public modules::Module
    {
    public:
      TormModule();
      ~TormModule();

      void shutdown();
      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);
      void* getPublicInterface();
      void handleEvent(u32 event, void* data);
      void module_main();

    private:
      TormInter *pInter;
      ModuleInterface* mInter;
      CTormInfo sInfo;

      int torPort;
      boost::thread connectControlLoop;
      bool running;

      bool parseModuleInfo();
      void connectControl();
    };
  };
};
