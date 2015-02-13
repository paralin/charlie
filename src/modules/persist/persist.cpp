#define MODULE_ID 2526948902
#include <modules/persist/Persist.h>
#include <boost/thread.hpp>

using namespace modules::persist;

PersistModule::PersistModule()
{
  MLOG("Persist module constructed...");
}

void PersistModule::shutdown()
{
  MLOG("Shutting down persist module..");
}

void PersistModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void PersistModule::injectDependency(u32 id, void* dep)
{
  MLOG("Dep injected "<<id);
}

void PersistModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
}

void* PersistModule::getPublicInterface()
{
  return NULL;
}

CHARLIE_CONSTRUCT(PersistModule);
