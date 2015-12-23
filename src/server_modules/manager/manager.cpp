#include <server_modules/manager/Manager.h>

using namespace server_modules::manager;

ManagerModule::ManagerModule() :
  mInter(NULL)
{
  MLOG("Manager module constructed...");
}

void ManagerModule::shutdown()
{
}

void ManagerModule::setModuleInterface(ServerModuleInterface* inter)
{
  mInter = inter;
}

void ManagerModule::inject(u32 id, void* dep)
{
}

void ManagerModule::release(u32 id)
{
}

void ManagerModule::handleEvent(u32 event, void* data)
{
}

void ManagerModule::module_main()
{
}

void ManagerModule::handleMessage(charlie::CMessageTarget* target, std::string& data)
{
}

u32 ManagerModule::getModuleId()
{
  return 3133916783;
}

ManagerModule::~ManagerModule()
{
}


CHARLIE_CONSTRUCT(ManagerModule);
