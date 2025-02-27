#pragma once
#include <string>
#include <set>
#include <IntTypes.h>
#include <protogen/charlie.pb.h>
#include <charlie/Crypto.h>
#include <charlie/SystemInfo.h>

#define VISIBLE __attribute__ ((visibility ("default")))

namespace modules
{
  class VISIBLE ModuleInterface
  {
    public:
      //Request that a module be loaded
      virtual void requireDependency(u32 id, bool optional = false) = 0;

      //Release a load request
      virtual void releaseDependency(u32 id) = 0;

      //Immediately commit changes
      virtual void requestModuleRecheck() = 0;

      //Attempts to verify and load a module table.
      //Rejects invalid / old module table.
      virtual void processModuleTable(const charlie::CModuleTable& tab) = 0;

      //Returns a copy of the verified module table data
      virtual charlie::CModuleTable* getModuleTable() = 0;

      //Get a mutable pointer to the config for this module
      virtual std::string* getStorage() = 0;

      //Get module info
      virtual std::string getModuleInfo() = 0;

      //Get module filename
      virtual std::string getModuleFilename(std::shared_ptr<charlie::CModule> mod) = 0;

      //Updates signature and saves storage.
      virtual void saveStorage(std::string& data) = 0;

      //Will unload and reload this module
      virtual void requestReload() = 0;

      //Return the crypto class
      virtual Crypto* getCrypto() = 0;

      //Recheck the module table
      virtual void triggerModuleRecheck() = 0;

      //Get system info
      virtual SystemInfo* getSysInfo() = 0;

      //Relocate all files to another place
      virtual int relocateEverything(const char* targetPath, const char* targetExecutableName) = 0;

      // Verify a module is loadable
      virtual bool moduleLoadable(u32 id) = 0;

      // Select a binary based on platform
      virtual charlie::CModuleBinary* selectBinary(std::shared_ptr<charlie::CModule> mod) = 0;

      // Select a module based on capabilities
      virtual std::vector<std::shared_ptr<charlie::CModule>> listModulesWithCap(u32 cap, bool filterHasBinary) = 0;
      virtual std::shared_ptr<charlie::CModule> selectModule(std::vector<std::shared_ptr<charlie::CModule>>& mods, charlie::CModuleBinary** bin = NULL) = 0;
      virtual std::shared_ptr<charlie::CModule> selectModule(u32 cap, charlie::CModuleBinary** bin = NULL) = 0;

      // List all the current instances
      virtual std::vector<charlie::CModuleInstance> listModuleInstances() = 0;
  };
};
