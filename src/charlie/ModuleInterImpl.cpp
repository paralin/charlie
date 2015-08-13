#include <charlie/ModuleInterImpl.h>
#include <charlie/ModuleManager.h>
#include <charlie/System.h>
#include <charlie/CryptoBuf.h>

using namespace modules;

ModuleInterImpl::ModuleInterImpl(ModuleManager * manager, ModuleInstance * inst)
{
  this->mManager = manager;
  this->inst = inst;
}

ModuleInterImpl::~ModuleInterImpl()
{
  this->mManager = NULL;
}

void ModuleInterImpl::requireDependency(u32 id)
{
  inst->modReqs.insert(id);
  if(mManager->moduleRunning(id))
     inst->notifyModuleLoaded(id, mManager->getModuleInstance(id)->publicInterface);
}

void ModuleInterImpl::releaseDependency(u32 id)
{
  inst->modReqs.erase(id);
}

void ModuleInterImpl::commitDepsChanges()
{
  mManager->deferRecheckModules();
}

charlie::CSignedBuffer* ModuleInterImpl::getModuleTable()
{
  if(!mManager->sys->config.has_emodtable()) return NULL;
  return mManager->sys->config.mutable_emodtable();
}

bool ModuleInterImpl::processModuleTable(charlie::CSignedBuffer* buf)
{
  if(buf == NULL) return false;
  return mManager->loadIncomingModuleTable(buf);
}

Crypto* ModuleInterImpl::getCrypto()
{
  return mManager->sys->crypto;
}

SystemInfo* ModuleInterImpl::getSysInfo()
{
  return &mManager->sys->sysInfo;
}

std::string* ModuleInterImpl::getStorage()
{
  charlie::CModuleStorage* stor = mManager->storageForModule(inst->module->id());
  if(!stor->has_buf()) return NULL;
  return stor->mutable_buf();
}

void ModuleInterImpl::saveStorage(std::string& data)
{
  charlie::CModuleStorage* stor = mManager->storageForModule(inst->module->id());
  stor->set_buf(data);
  mManager->deferSaveConfig();
}

void ModuleInterImpl::requestReload()
{
  u32 id = inst->module->id();
  CLOG("Module requested reload "<<id<<"...");
  mManager->deferReloadModule(id);;
}

void ModuleInterImpl::triggerModuleRecheck()
{
  CLOG("Module requested recheck "<<inst->module->id()<<"...");
  mManager->deferRecheckModules();
}

int ModuleInterImpl::relocateEverything(const char* targetPath, const char* targetExecutableName)
{
  return mManager->sys->relocateEverything(targetPath, targetExecutableName);
}

std::string ModuleInterImpl::getModuleInfo()
{
  bool hasInfo = inst->module->has_info();
  if(!hasInfo) return std::string();
  return inst->module->info();
}

std::string ModuleInterImpl::getModuleFilename(charlie::CModule* mod)
{
  char* fn = mManager->getModuleFilename(mod);
  std::string res(fn);
  free(fn);
  return res;
}

bool ModuleInterImpl::moduleLoadable(u32 id)
{
  return mManager->moduleLoadable(id, false);
}
