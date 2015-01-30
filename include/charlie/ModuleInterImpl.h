#pragma once
#include <ModuleInterface.h>

class ModuleManager;
class ModuleInstance;
class VISIBLE ModuleInterImpl : public modules::ModuleInterface
{
  public:
    ModuleInterImpl(ModuleManager *manager, ModuleInstance * inst);
    ~ModuleInterImpl();

    //Request that a module be loaded
    int  requireDependency(u32 id);
    void releaseDependency(u32 id);

    //Immediately commit changes
    void commitDepsChanges();

    //Attempts to verify and load a module table.
    //Rejects invalid / old module table.
    bool processModuleTable(charlie::CSignedBuffer* buf);

    //Returns a copy of the verified module table data
    charlie::CSignedBuffer* getModuleTable();

    Crypto* getCrypto();

  private:
    ModuleManager *mManager;
    ModuleInstance * inst;
};
