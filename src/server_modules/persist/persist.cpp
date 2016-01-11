#include <server_modules/persist/Persist.h>

using namespace server_modules::persist;

PersistModule::PersistModule() :
  mInter(NULL)
{
  MLOG("Persist module constructed...");
}

void PersistModule::shutdown()
{
}

void PersistModule::setModuleInterface(SModuleInterface* inter)
{
  mInter = inter;
}

void PersistModule::inject(u32 id, void* dep)
{
}

void PersistModule::release(u32 id)
{
}

void PersistModule::handleEvent(u32 event, void* data)
{
}

void PersistModule::module_main()
{
}

void PersistModule::handleMessage(charlie::CMessageTarget* target, std::string& data)
{
}

u32 PersistModule::getModuleId()
{
  return 2526948902;
}

PersistModule::~PersistModule()
{
}


CHARLIE_CONSTRUCT(PersistModule);
