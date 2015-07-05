
#pragma once
#include <string>
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
      virtual void requireDependency(u32 id) = 0;
      virtual void releaseDependency(u32 id) = 0;

      //Immediately commit changes
      virtual void commitDepsChanges() = 0;

      //Attempts to verify and load a module table.
      //Rejects invalid / old module table.
      virtual bool processModuleTable(charlie::CSignedBuffer* buf) = 0;

      //Returns a copy of the verified module table data
      virtual charlie::CSignedBuffer* getModuleTable() = 0;

      //Get a mutable pointer to the config for this module
      virtual std::string* getStorage() = 0;

      //Get module info
      virtual std::string getModuleInfo() = 0;

      //Get module filename
      virtual std::string getModuleFilename(charlie::CModule* mod) = 0;

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
      virtual int relocateEverything(const char* targetPath) = 0;

      virtual bool moduleLoadable(u32 id) = 0;
  };
};
