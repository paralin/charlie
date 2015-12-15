#include <Module.h>
#include <modules/directconnect/DirectConnect.h>
#include <boost/thread.hpp>

using namespace modules::directconnect;

DirectConnectModule::DirectConnectModule()
{
  MLOG("Direct connect socket net module constructed...");
  pInter = new DirectConnectInter(this);
}

DirectConnectModule::~DirectConnectModule()
{
  delete pInter;
}

void DirectConnectModule::shutdown()
{
  MLOG("Shutting down directconnect module..");
}

void DirectConnectModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void DirectConnectModule::injectDependency(u32 id, void* dep)
{
  if(!dep || !id) return;
  MLOG("Dep injected "<<id);
  if (id == 3133916783)
  {
    manager = (modules::manager::ManagerInter*) dep;
    managerMtx.unlock();
  }
}

void DirectConnectModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
  if (id == 3133916783)
  {
    manager = NULL;
    managerMtx.lock();
  }
}

void* DirectConnectModule::getPublicInterface()
{
  return pInter;
}

// Main function
void DirectConnectModule::module_main()
{
  MLOG("Waiting for manager module...");
  managerMtx.lock();
  managerMtx.unlock();

  populateServerKeys();
}

void DirectConnectModule::populateServerKeys()
{
  if (manager == NULL) return;
  modules::manager::CManagerInfo* info = manager->getInfo();
  if (info == NULL) return;
  for (int i = 0; i < info->server_key_size(); i++)
    serverKeys.insert(info->server_key(i));
}

void DirectConnectModule::handleEvent(u32 eve, void* data)
{
}

DirectConnectInter::DirectConnectInter(DirectConnectModule * mod)
{
  this->mod = mod;
}

bool DirectConnectInter::ready()
{
  return false;
}


DirectConnectInter::~DirectConnectInter()
{
}

void DirectConnectInter::handleCommand(void* command)
{
}

CHARLIE_CONSTRUCT(DirectConnectModule);
