#include <charlie/ModuleInterImpl.h>
#include <charlie/ModuleManager.h>
#include <charlie/System.h>

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

//todo threadsafe
int ModuleInterImpl::requireDependency(u32 id)
{
  inst->modReqs.insert(id);
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
