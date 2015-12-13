#define MODULE_ID 2777954855
#include <Module.h>
#include <modules/client/Client.h>
#include <boost/thread.hpp>

using namespace modules::client;

ClientModule::ClientModule()
{
  MLOG("Client module constructed...");
  pInter = new ClientInter(this);
  manager = NULL;
  netModule = NULL;
  netModuleId = 0;
  // Lock until it's ready
  managerInterMtx.lock();
}

ClientModule::~ClientModule()
{
  delete pInter;
}

void ClientModule::shutdown()
{
  MLOG("Shutting down client module..");
}

void ClientModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;

  // Depend on the manager
  mInter->requireDependency(3133916783);
}

void ClientModule::injectDependency(u32 id, void* dep)
{
  if(!dep || !id) return;
  MLOG("Dep injected "<<id);
  if (id == netModuleId)
  {
    MLOG("Network module loaded.");
    netModule = (modules::ModuleNet*)dep;
    return;
  }
  switch(id)
  {
    case (u32)3133916783:
      managerInterMtx.unlock();
      manager = (modules::manager::ManagerInter*)dep;
      break;
  }
}

void ClientModule::releaseDependency(u32 id)
{
  switch(id)
  {
    case (u32)3133916783:
      managerInterMtx.lock();
      manager = NULL;
      break;
  }

  if (id == netModuleId)
    netModule = NULL;

  MLOG("Dep released "<<id);
}

void* ClientModule::getPublicInterface()
{
  return pInter;
}

// Main function
void ClientModule::module_main()
{
  MLOG("Waiting for manager to be injected...");

  // We need to wait for the manager to be injected
  managerInterMtx.lock();
  managerInterMtx.unlock();

  selectNetworkModule();
}

void ClientModule::handleEvent(u32 eve, void* data)
{
  charlie::EModuleEvents event = (charlie::EModuleEvents) eve;
  if (event == charlie::EVENT_MODULE_TABLE_RELOADED)
      selectNetworkModule();
}

void ClientModule::selectNetworkModule()
{
  if (mInter == NULL)
    return;

  // Dont mess with a working connection
  if (netModule != NULL && netModule->ready())
    return;

  // See if there's a better module
  charlie::CModule* tmod = mInter->selectModule(charlie::MODULE_CAP_NET, NULL);
  if (tmod == NULL)
  {
    MLOG("Waiting for a suitable network module...");
    return;
  }

  if (tmod->id() == netModuleId)
    return;

  // Select this module
  if (netModuleId != 0)
  {
    MLOG("Releasing module " << netModuleId);
    mInter->releaseDependency(netModuleId);
  }

  netModule = NULL;
  netModuleId = tmod->id();
  MLOG("Selecting network module " << netModuleId);
  mInter->requireDependency(netModuleId);
}

ClientInter::ClientInter(ClientModule * mod)
{
  this->mod = mod;
}

ClientInter::~ClientInter()
{
}

void ClientInter::handleCommand(void* command)
{
}

CHARLIE_CONSTRUCT(ClientModule);
