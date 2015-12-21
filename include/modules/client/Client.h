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

namespace modules
{
  namespace client
  {
    using boost::asio::ip::tcp;

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
      ModuleInterface* mInter;
      ClientInter *pInter;
      CClientInfo sInfo;
      modules::manager::ManagerInter* manager;

      std::set<std::string> serverKeys;
      boost::mutex managerMtx;
      std::set<std::string> server;
      std::vector<unsigned char> buf;

      void populateServerKeys();
      int parseModuleInfo();

      void initNetworking();
      void releaseNetworking();
      void disconnect();
      void resetReceiveContext();
      void unexpectedDataReceived();
      void handleMessage(std::string& data);

      void selectNetworkModule();

      void send(charlie::EMsg emsg, u32 target, std::string& data, u32 jobid = 0, u32 targetEmsg = 0);
      void send(u32 targetModule, u32 jobId, u32 targetEmsg, std::string& data);

      void sendClientIdentify();
      void sendClientAccept(bool sendInfo = false);

      bool tryConnectAllEndpoints();
      bool tryConnectNetModules();
      bool tryConnectEndpoint(const char* endp);

      bool validateMessageHeader();
      bool validateMessageBody();
      bool handleServerIdentify();

      void handleServerAccept(std::string& data);

      boost::asio::io_service* io_service;
      tcp::resolver* resolver;
      tcp::socket* socket;

      bool running;
      bool connected;
      bool wasConnected;

      // State for the connection
      bool expectingHeaderLengthPrefix;
      u32 expectedHeaderSize;
      bool expectingHeader;
      u32 expectedBodySize;
      bool handshakeComplete;
      charlie::CMessageHeader head;
      charlie::CMessageBody body;
      std::time_t timeConnected;

      // Value to hash in identify
      std::string serverChallenge;
      std::string clientChallenge;
      Crypto* sessionCrypto;
      Crypto* systemCrypto;

      // Handshake phases
      bool receivedServerIdentify;
      bool sentClientIdentify;

      // Sub-networking module
      u32 netModuleId;
      bool usingNetModule;

      std::map<u32, ModuleNet*> loadedNetModules;
      std::set<u32> netModuleIds;

      std::map<u32, ModuleAPI*> loadedModules;

      // Support socks later
    };
  };
};
