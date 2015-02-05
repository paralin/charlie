
#pragma once
#include <IntTypes.h>
#include <protogen/charlie.pb.h>
#include <charlie/Crypto.h>

#define VISIBLE __attribute__ ((visibility ("default")))

namespace modules
{
  class VISIBLE ModuleInterface
  {
    public:
      //Request that a module be loaded
      virtual int  requireDependency(u32 id) = 0;
      virtual void releaseDependency(u32 id) = 0;

      //Immediately commit changes
      virtual void commitDepsChanges() = 0;

      //Attempts to verify and load a module table.
      //Rejects invalid / old module table.
      virtual bool processModuleTable(charlie::CSignedBuffer* buf) = 0;

      //Returns a copy of the verified module table data
      virtual charlie::CSignedBuffer* getModuleTable() = 0;

      //Get a mutable pointer to the config for this module
      virtual charlie::CModuleStorage* getStorage() = 0;

      //Updates signature and saves storage.
      virtual void saveStorage(void* data, size_t len) = 0;

      //Will unload and reload this module
      virtual void requestReload() = 0;

      //Return the crypto class
      virtual Crypto* getCrypto() = 0;
  };
};
