#pragma once

#include <Common.h>
#include <Logging.h>
#include <IntTypes.h>

#include <ServerModule.h>
#include <charlie/Crypto.h>
#include <protogen/charlie_net.pb.h>
#include <protogen/client.pb.h>

#include <mongo/bson/bson.h>
#include <mongo/client/init.h>
#include <mongo/client/dbclient.h>

namespace server_modules
{
  namespace mongo
  {
    class VISIBLE MongoModule : public server_modules::ServerModule
    {
    public:
      MongoModule();
      ~MongoModule();

      void shutdown();
      void setModuleInterface(SModuleInterface* inter);
      void inject(u32 id, void* dep);

      void release(u32 id);
      void handleEvent(u32 event, void* data);
      void module_main();
      void handleMessage(charlie::CMessageTarget* target, std::string& data);
      u32  getModuleId();

      VISIBLE void initWithInfo(modules::client::CClientSystemInfo& info);
      VISIBLE void updateModuleStates(modules::client::CClientModuleState& info);
      void setOnlineState();
      void setOfflineState();

    private:
      SModuleInterface* mInter;

      bool connected;
      std::shared_ptr<::mongo::DBClientBase> conn;
      static bool mongoInited;
      boost::mutex connMtx;
      boost::mutex clientMtx;

      std::string dbname;
      std::string clientsNs;
      bool inited;
      bool isShutdown;
    };
  };
};
