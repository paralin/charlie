#pragma once

#include <Common.h>
#include <Logging.h>
#include <IntTypes.h>

#include <SModuleInterface.h>
#include <charlie/Crypto.h>
#include <server_modules/mongo/Mongo.h>

namespace server_modules
{
  namespace client
  {
    class ClientModule : public server_modules::ServerModule
    {
    public:
      ClientModule();
      ~ClientModule();

      void shutdown();
      void setModuleInterface(SModuleInterface* inter);
      void inject(u32 id, void* dep);
      void release(u32 id);
      void handleEvent(u32 event, void* data);
      void module_main();
      void handleMessage(charlie::CMessageTarget* target, std::string& data);
      u32  getModuleId();

    private:
      SModuleInterface* mInter;
      Crypto* sessionCrypto;
      Crypto* systemCrypto;
      bool clientInfoReceived;
      bool clientModulesReceived;
      bool isShutdown;
      mongo::MongoModule* mongoMod;
    };
  };
};
