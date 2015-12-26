#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>

#include "TormInter.h"

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <modules/manager/ManagerInter.h>
#include <modules/client/ClientInter.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <protogen/torm.pb.h>

#include <boost/thread.hpp>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

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

      bool running;
      bool connected;
      bool inited;

      void newIdentity();
      tcp::socket* getSocket(std::time_t* timeConnected);

    private:
      TormInter *pInter;
      ModuleInterface* mInter;
      CTormInfo sInfo;
      modules::manager::ManagerInter* manager;
      modules::client::ClientInter* client;

      int torPort;
      std::string socksUsername;
      std::string socksPassword;

      boost::thread connectControlLoop;

      bool parseModuleInfo();
      void connectControl();
      bool tryConnectAllEndpoints();
      bool tryConnectEndpoint(std::string& endp);
      bool tryEstablishSocksConnection();

      void handleEndpointFailure(unsigned int endpHash, unsigned char err);

      boost::asio::io_service io_service;
      tcp::resolver resolver;
      tcp::socket socket;

      std::time_t timeConnected;
      std::map<unsigned int, std::time_t> endpointTimeouts;
    };
  };
};
