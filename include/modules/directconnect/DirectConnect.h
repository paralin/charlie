#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>

#include <set>
#include <vector>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "DirectConnectInter.h"
#include <ModuleNet.h>
#include <modules/manager/ManagerInter.h>
#include <protogen/directconnect.pb.h>

namespace modules
{
  namespace directconnect
  {
    using boost::asio::ip::tcp;

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
      CDirectConnectInfo sInfo;
      modules::manager::ManagerInter* manager;

      std::set<std::string> serverKeys;
      boost::mutex managerMtx;
      std::set<std::string> server;

      void populateServerKeys();
      int parseModuleInfo();

      void initNetworking();
      void releaseNetworking();
      void disconnect();
      void resetReceiveContext();
      void unexpectedDataReceived();

      bool tryConnectAllEndpoints();
      bool tryConnectEndpoint(const char* endp);

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

      // Support socks later
    };
  };
};
