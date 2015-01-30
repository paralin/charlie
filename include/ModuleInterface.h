
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
      virtual int  requireDependency(u32 id);
      virtual void releaseDependency(u32 id);

      //Immediately commit changes
      virtual void commitDepsChanges();

      //Attempts to verify and load a module table.
      //Rejects invalid / old module table.
      virtual bool processModuleTable(charlie::CSignedBuffer* buf);

      //Returns a copy of the verified module table data
      virtual charlie::CSignedBuffer* getModuleTable();

      virtual Crypto* getCrypto();
  };
};
