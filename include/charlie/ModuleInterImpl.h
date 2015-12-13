#pragma once
#include <string>
#include <ModuleInterface.h>
#include <charlie/SystemInfo.h>

class ModuleManager;
class ModuleInstance;
class ModuleInterImpl : public modules::ModuleInterface
{
  public:
    ModuleInterImpl(ModuleManager *manager, ModuleInstance * inst);
    ~ModuleInterImpl();

    //Request that a module be loaded
    void requireDependency(u32 id);
    void releaseDependency(u32 id);

    //Immediately commit changes
    void commitDepsChanges();

    //Attempts to verify and load a module table.
    //Rejects invalid / old module table.
    bool processModuleTable(charlie::CSignedBuffer* buf);

    //Returns a copy of the verified module table data
    charlie::CSignedBuffer* getModuleTable();

    std::string getModuleFilename(charlie::CModule* mod);

    Crypto* getCrypto();
    SystemInfo* getSysInfo();

    std::string* getStorage();

    //Updates signature and saves storage.
    void saveStorage(std::string& data);

    //Will unload and reload this module
    void requestReload();

    //Trigger a recheck of the modules
    void triggerModuleRecheck();

    int relocateEverything(const char* targetPath, const char* targetExecutableName);

    std::string getModuleInfo();

    bool moduleLoadable(u32 id);

    charlie::CModuleBinary* selectBinary(charlie::CModule* mod);

    charlie::CModule* selectModule(u32 cap, charlie::CModuleBinary** bin = NULL);

  private:
    ModuleManager *mManager;
    ModuleInstance * inst;
};
