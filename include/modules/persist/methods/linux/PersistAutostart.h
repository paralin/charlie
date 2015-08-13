#pragma once
#include <Common.h>
// Supports linux only
#ifdef CHARLIE_LINUX
#define CHARLIE_PersistAutostart_AVAILABLE
#include <Logging.h>
#include "../../PersistInter.h"
#include "../../PersistMethod.h"
#include <boost/filesystem.hpp>

/*
 * Persist Home
 * ============
 *
 * OS: Linux
 * Uses the ~/.config/autostart folder to auto-start on login.
 */
namespace modules
{
  namespace persist
  {
    class PersistModule;
    class PersistAutostart : public PersistMethod
    {
    public:
      PersistAutostart();
      ~PersistAutostart();

      void init(ModuleInterface *inter, PersistModule* persist);
      bool inUse();
      bool canUse();
      bool setup();
      void cleanup();

    private:
      ModuleInterface* inter;
      PersistModule* persist;
      boost::filesystem::path homePath;
      boost::filesystem::path configPath;
      boost::filesystem::path storagePath;
      boost::filesystem::path gnomeConfigPath;
      boost::filesystem::path kdeConfigPath;
      boost::filesystem::path kdeAutoConfigPath;
      boost::filesystem::path gnomeConfigFilePath;
      boost::filesystem::path kdeConfigFilePath;
      std::string exeName;
    };
  };
};
#endif
