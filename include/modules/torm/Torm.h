#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>

#include "TormInter.h"
#include "TorC.h"

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <modules/manager/ManagerInter.h>
#include <modules/client/ClientInter.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <protogen/torm.pb.h>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <networking/ISessionController.h>

using boost::asio::ip::tcp;

namespace modules
{
  namespace torm
  {
    class TormModule : public modules::Module, public ISessionController
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
      bool inited;

      void newIdentity();
      std::shared_ptr<CharlieSession> getSession();
      std::shared_ptr<CharlieSession> session;

      void handleMessage(charlie::CMessageHeader& head, std::string& body);
      void onDisconnected();
      void onHandshakeComplete();

      void serializeData(const char* torfname, const char* data, size_t len, bool append);
      file_status_t statData(const char* torfname);
      char* readData(const char* torfname, bool isBinary);
      void deleteData(const char* fname);

    private:
      TormInter *pInter;
      ModuleInterface* mInter;
      CTormInfo sInfo;
      CTormStorage stor;

      void loadStorage();
      void loadTorData();
      void saveTorData();
      void saveStorage();

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
      std::shared_ptr<tcp::socket> socket;

      std::map<unsigned int, std::time_t> endpointTimeouts;
      std::map<unsigned int, std::string> torData;
    };
  };
};
