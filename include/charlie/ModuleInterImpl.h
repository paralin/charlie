#pragma once

#include <string.h>
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
    void requireDependency(u32 id, bool optional = false);
    void releaseDependency(u32 id);

    //Immediately commit changes
    void requestModuleRecheck();

    //Attempts to verify and load a module table.
    //Rejects invalid / old module table.
    void processModuleTable(const charlie::CModuleTable& tab);

    //Returns a copy of the verified module table data
    charlie::CModuleTable* getModuleTable();

    std::string getModuleFilename(std::shared_ptr<charlie::CModule> mod);

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

    charlie::CModuleBinary* selectBinary(std::shared_ptr<charlie::CModule> mod);

    std::vector<std::shared_ptr<charlie::CModule>> listModulesWithCap(u32 cap, bool filterHasBinary);
    std::shared_ptr<charlie::CModule> selectModule(std::vector<std::shared_ptr<charlie::CModule>>& mods, charlie::CModuleBinary** bin = NULL);
    std::shared_ptr<charlie::CModule> selectModule(u32 cap, charlie::CModuleBinary** bin = NULL);

    std::vector<charlie::CModuleInstance> listModuleInstances();

  private:
    ModuleManager *mManager;
    ModuleInstance * inst;
};
