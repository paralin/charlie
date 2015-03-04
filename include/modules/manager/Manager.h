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
      charlie::CModuleTable* fetchStaticModTable(charlie::CSignedBuffer** lmb=0);
      int parseModuleInfo();
      void loadStorage();
      void saveStorage();
      std::string fetchUrl(const std::string& url);
      std::string fetchStaticUrl(const std::string& url);
      std::string fetchOnionCabUrl(const std::string& url);
      int updateTableFromInternet(charlie::CModuleTable **wtbl = 0);
      int downloadModules(charlie::CModuleTable* table = 0);

      ModuleInterface* mInter;
      CManagerInfo sInfo;
      ManagerInter* pInter;
      boost::network::http::client client;
      CManagerStorage stor;
      Crypto* crypt;

      std::string onionCabHash;
    };
  };
};
