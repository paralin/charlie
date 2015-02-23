#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <protogen/manager.pb.h>
#define BOOST_NETWORK_ENABLE_HTTPS
#include <boost/network/protocol/http.hpp>
#include "ManagerInter.h"

namespace modules
{
  namespace manager
  {
    class ManagerModule : public modules::Module
    {
    public:
      //Only constructor. Destructor doesn't work.
      ManagerModule();
      void shutdown();

      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);

      void module_main();

      void* getPublicInterface();

      //Nonstandard
    private:
      int fetchStaticModTable();
      int parseModuleInfo();
      std::string fetchUrl(const std::string& url);
      std::string fetchStaticUrl(const std::string& url);
      std::string fetchOnionCabUrl(const std::string& url);

      ModuleInterface* mInter;
      charlie::CModuleStorage* stor;
      CManagerInfo sInfo;
      ManagerInter* pInter;
      boost::network::http::client client;

      std::string onionCabHash;
    };
  };
};
