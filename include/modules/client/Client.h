#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>

#include <set>
#include <vector>

#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "ClientInter.h"
#include <ModuleNet.h>
#include <modules/manager/ManagerInter.h>

#include <protogen/charlie.pb.h>
#include <protogen/client.pb.h>

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <networking/CharlieSession.h>

namespace modules
{
  namespace client
  {
    using boost::asio::ip::tcp;

    class ClientModule : public modules::Module, public ISessionController
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

      void sendSystemInfo();
      void retryConnectionsNow();
      void unexpectedDataReceived();
      void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL);
      void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data);

    private:
      ModuleInterface* mInter;
      CClientInfo sInfo;
      std::shared_ptr<ClientInter> pInter;
      modules::manager::ManagerInter* manager;
      Crypto* systemCrypto;

      boost::mutex managerMtx;
      std::set<std::string> serverKeys;

      void populateServerKeys();
      int parseModuleInfo();

      void handleMessage(charlie::CMessageHeader& head, std::string& data);

      void selectNetworkModule();
      void dependAllModules();

      bool tryConnectAllEndpoints();
      bool tryConnectNetModules();
      bool tryConnectEndpoint(const char* endp);

      boost::asio::io_service io_service;
      tcp::resolver resolver;
      std::shared_ptr<tcp::socket> pendingSocket;
      std::shared_ptr<CharlieSession> session;

      bool running;
      bool isDirect;

      // Sub-networking module
      u32 netModuleId;
      bool usingNetModule;
      ModuleNet* torModule;
      std::time_t nextConnectAttempt;

      std::map<u32, ModuleAPI*> loadedModules;
      std::map<u32, ModuleNet*> loadedNetModules;
      std::set<u32> netModuleIds;

      void onDisconnected();
      void onHandshakeComplete();
    };
  };
};
